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
                if (HeaderInfoStringParser(token) == FALSE)
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

    int GetShortDirNameStartPos(TFileInfo<TChar>& FileInfo)
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




};



