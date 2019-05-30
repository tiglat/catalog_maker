#pragma once

#include <string>

#include "windows.h"

#include "GlobalVars.h"
#include "IStringOperations.h"

template <typename TChar, typename TString>
class CatalogReader
{
private: 
    static const USHORT TEXT_LINE_LENGTH = 1000;
    static const USHORT LIST_COLUMN_NUMBER = 6;

    HANDLE _CatalogFile;
    IStringOperations<TChar, TString> *_pStringOperations;

    TChar		_pBuf[TEXT_LINE_LENGTH + 1] = { 0 };

    // Directory path without root dir.
    TChar		_DirectoryShortName[TEXT_LINE_LENGTH + 1] = { 0 };

    // Beginning position of short dir name. Short dir name is a part of path from root of file list.
    // For example, list file was created for files and subdirs of c:\somedir1\somedir2\somedir3.
    // c:\somedir1\somedir2 is a root directory for this list; somedir3 - short dir name
    USHORT		_ShortDirNamePos = 0xFFFF;

    USHORT		_iReadPos = 0;
    USHORT		_iWritePos = 0;
    BOOL		_bNeedData = true;
    UCHAR		_IsHeader = 2;
    DWORD		_ReturnedLength = 0;
    TFileInfo<TChar> _CurrentFileInfo;

    using TColumnHandlerPtr = void (CatalogReader::*)(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo);
    
    struct TListInfo 
    {
        TColumnHandlerPtr	HandleFunc;
        USHORT              StartIdx;
        USHORT              Len;
    };

    TListInfo _ListInfo[LIST_COLUMN_NUMBER] = {0};

public:
    
    CatalogReader(HANDLE CatalogFileHandle, IStringOperations<TChar, TString> *pOps) 
        : _CatalogFile(CatalogFileHandle), _pStringOperations(pOps)
    {
    }

    virtual ~CatalogReader()
    {}

    int ReadNext(TFileInfo<TChar>& FileInfo)
    {
        const TChar seps = 0x0A;
        TChar* token;
        int	rv;
        USHORT  len;

        memset(&FileInfo, 0, sizeof(FileInfo));

        while (1)
        {
            // if we don't have data for analyse
            // we should read next block
            if (_bNeedData)
            {
                rv = ReadDataBlock();

                if (rv != SUCCESS)
                {
                    return(rv);
                }

                _bNeedData = FALSE;
            }

            // get next string from read buffer
            token = _pStringOperations->StrTok(&_pBuf[_iReadPos], &seps);

            // if there is no data for analyse 
            // then we should read next block
            if (token == NULL)
            {
                _bNeedData = TRUE;
                _iReadPos = 0;
                _iWritePos = 0;
                continue;
            }

            // change read position in the buffer
            len = (USHORT)_pStringOperations->StrLen(token);
            _iReadPos += len + 1;

            // is it the end of read data block?
            // first condition to find the end of file - every token must have 0x0D at the end 
            // except last one It is needed to skip "total files and size" string
            // second condition to find end of current block of data
            if (token[len - 1] != 0x0D || _iReadPos > _ReturnedLength + _iWritePos)
            {
                // if token is not complete string
                // we should store this token and read next block
                memcpy(_pBuf, token, len);
                _iWritePos = len;
                _iReadPos = 0;
                _bNeedData = TRUE;
                continue;
            }

            // skip empty strings
            if (len == 1 && token[0] == 0x0D)
            {
                continue;
            }

            // Analyse string if it is correct
            if (_IsHeader)
            {
                if (ParseListHeader(token) == FALSE)
                {
                    return (E_BAD_ARCHIVE);
                }
                continue;
            }
            else
            {
                FileInfoStringParser(token, &FileInfo);
            }

            _CurrentFileInfo = FileInfo;

            GetShortDirName(FileInfo);

            break;
        }

        return SUCCESS;
    }

    void GetFullFileName(TFileInfo<TChar>& FileInfo, TChar* FullFileName)
    {
        // if file is directory copy its name 
        if (FileInfo.Attr & 0x10)
        {
            _pStringOperations->StrCpy(FullFileName, _DirectoryShortName);
        }
        else
        {
            // if file is file :)  build full name for that file
            _pStringOperations->StrCpy(FullFileName, _DirectoryShortName);
            // get short file name
            TChar* pstr = _pStringOperations->StrRChr(FileInfo.Name, '\\');
            if (pstr)
            {
                _pStringOperations->StrCat(FullFileName, ++pstr);
            }
            else
            {
                _pStringOperations->StrCat(FullFileName, FileInfo.Name);
            }
        }
    }

private:

    /*****************************************************************************
        Routine:     ReadDataBlock
    ------------------------------------------------------------------------------
        Description:

        Arguments:

        Return Value:


    *****************************************************************************/

    int ReadDataBlock()
    {
        int rv;

        rv = ReadFile(
            _CatalogFile,
            &_pBuf[_iWritePos],
            TEXT_LINE_LENGTH - _iWritePos,
            &_ReturnedLength,
            NULL
        );

        if (rv &&  _ReturnedLength == 0)
        {
            return E_END_ARCHIVE;
        }

        if (rv == NULL)
        {
            return E_EREAD;
        }

        return SUCCESS;
    }


    /*****************************************************************************
    Routine:     GetShortDirName
    ------------------------------------------------------------------------------
    Description:
        Extract short directory name from a full directory name. 
        Remember position of beginning of short dir name.

    Arguments:
        FileInfo   - structure contained the name to parse

    Return Value:

    *****************************************************************************/

    void GetShortDirName(TFileInfo<TChar>& FileInfo)
    {
        if (_ShortDirNamePos == 0xFFFF)
        {
            _ShortDirNamePos = GetShortDirNameStartPos(FileInfo);
        }

        _pStringOperations->StrCpy(_DirectoryShortName, &FileInfo.Name[_ShortDirNamePos]);
    }

    /*****************************************************************************
    Routine:     GetShortDirNameStartPos
    ------------------------------------------------------------------------------
    Description:
        Calculates start position of short dir name (see explanation above)

    Arguments:
        FileInfo   - structure contained the name to parse

    Return Value:

    *****************************************************************************/

    size_t GetShortDirNameStartPos(TFileInfo<TChar>& FileInfo)
    {
        TChar* pstr;

        // Check if dir name is full
        pstr = _pStringOperations->StrChr(FileInfo.Name, ':');

        // If it is full, then extract the short dir name
        if (pstr)
        {
            // if file name is full then we need to cut root dir
            USHORT len = (USHORT)_pStringOperations->StrLen(FileInfo.Name);
            FileInfo.Name[len - 1] = 0;
            pstr = _pStringOperations->StrRChr(FileInfo.Name, '\\');
            FileInfo.Name[len - 1] = '\\';

            if (pstr)
            {
                return _pStringOperations->StrLen(FileInfo.Name) - _pStringOperations->StrLen(++pstr);

            }
        }
        
        return 0;
    }

    /*****************************************************************************
        Routine:     ParseListHeader
    ------------------------------------------------------------------------------
        Description:
                    Analyse header of list file and fill array of column
                    descriptors which will use to read list file on the next steps

        Arguments:
                    pStr	-	pointer to string from list file

        Return Value:
                    If this string matches list file format than TRUE
                    will be returned

    *****************************************************************************/

    BOOL ParseListHeader(TChar* pStr)
    {
        TChar* Offset;

        if (_IsHeader == 2) // first line of the header
        {
            //======= find and handle column "File name" ============
            Offset = _pStringOperations->GetFileNameColumnPtr(pStr);

            if (Offset == NULL || Offset != pStr)
            {
                return FALSE;
            }

            _ListInfo[COL_NAME].StartIdx = 1;
            _ListInfo[COL_NAME].HandleFunc = &CatalogReader::FileNameColumnHandler;

            //======= find and handle column "Ext" ============
            Offset = _pStringOperations->GetExtColumnPtr(pStr);

            _ListInfo[COL_EXT].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ListInfo[COL_EXT].HandleFunc = &CatalogReader::ExtColumnHandler;
            _ListInfo[COL_NAME].Len =
                Offset ?
                (USHORT)(Offset - pStr) :
                0;

            //======= find and handle column "Size" ============
            Offset = _pStringOperations->GetSizeColumnPtr(pStr);

            _ListInfo[COL_SIZE].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ListInfo[COL_SIZE].HandleFunc = &CatalogReader::SizeColumnHandler;
            _ListInfo[COL_SIZE].Len = 15;

            // if Size field exist and File name column width 
            // was not calculated yet
            if (Offset && _ListInfo[COL_NAME].Len == 0)
            {
                _ListInfo[COL_NAME].Len = (USHORT)(Offset - pStr);
            }

            // if Size field exist and Ext column width 
            // was not calculated yet
            if (Offset && _ListInfo[COL_EXT].Len == 0)
            {
                _ListInfo[COL_EXT].Len =
                    (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1);
            }

            //======= find and handle column "Date" ============
            Offset = _pStringOperations->GetDateColumnPtr(pStr);

            _ListInfo[COL_DATE].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ListInfo[COL_DATE].HandleFunc = &CatalogReader::DateColumnHandler;
            _ListInfo[COL_DATE].Len = 10;

            if (Offset && _ListInfo[COL_NAME].Len == 0)
            {
                _ListInfo[COL_NAME].Len = (USHORT)(Offset - pStr);
            }

            if (Offset && _ListInfo[COL_EXT].Len == 0)
            {
                _ListInfo[COL_EXT].Len =
                    (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1);
            }

            //======= find and handle column "Time" ============
            Offset = _pStringOperations->GetTimeColumnPtr(pStr);

            _ListInfo[COL_TIME].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ListInfo[COL_TIME].HandleFunc = &CatalogReader::TimeColumnHandler;
            _ListInfo[COL_TIME].Len = 8;

            if (Offset && _ListInfo[COL_NAME].Len == 0)
            {
                _ListInfo[COL_NAME].Len = (USHORT)(Offset - pStr);
            }

            if (Offset && _ListInfo[COL_EXT].Len == 0)
            {
                _ListInfo[COL_EXT].Len =
                    (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1);
            }

            //======= find and handle column "Attr" ============
            Offset = _pStringOperations->GetAttrColumnPtr(pStr);

            _ListInfo[COL_ATTR].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ListInfo[COL_ATTR].HandleFunc = &CatalogReader::AttrColumnHandler;
            _ListInfo[COL_ATTR].Len = 4;

            _ListInfo[COL_NAME].Len =
                _ListInfo[COL_NAME].Len ?
                _ListInfo[COL_NAME].Len :
                (Offset ? (USHORT)(Offset - pStr) : (USHORT)strlen(pStr));

            _ListInfo[COL_EXT].Len =
                _ListInfo[COL_EXT].Len ?
                _ListInfo[COL_EXT].Len :
                (Offset ?
                (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1) :
                    (USHORT)strlen(pStr) - _ListInfo[COL_EXT].StartIdx - 1);
        }

        if (_IsHeader == 1) // second and the last line of the header
        {
            // wrong file
            if (_ListInfo[COL_NAME].StartIdx == 0)
                return FALSE;
        }

        _IsHeader--;

        return TRUE;
    }

    void FileNameColumnHandler(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo)
    {

    }

    void ExtColumnHandler(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo)
    {

    }

    void SizeColumnHandler(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo)
    {

    }

    void DateColumnHandler(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo)
    {

    }

    void TimeColumnHandler(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo)
    {

    }

    void AttrColumnHandler(TChar* pStr, USHORT StrLen, TFileInfo<TChar>* pFileInfo)
    {

    }
};





