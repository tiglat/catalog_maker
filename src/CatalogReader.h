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

    // Length of path to root directory which was listed in the list file.
    // For example, list file has content of c:\somedir1\somedir2.
    // somedir2 is a root directory for this list.
    // Root dir is always the first item in a list.
    USHORT		_RootDirLen = 0xFFFF;

    USHORT		_iReadPos = 0;
    USHORT		_iWritePos = 0;
    BOOL		_bNeedData = true;
    UCHAR		_iThisIsHeader = 2;
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
        TChar seps = 0x0A;
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
            token = _pStringOperations->StrTok(&_pBuf[_iReadPos], seps);

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
            if (_iThisIsHeader)
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

            // if current file is directory than store its short name
            // to use in future to make full file name
            if (FileInfo.Attr & 0x10)
            {
                if (_RootDirLen == 0)
                {
                    GetShortDirName(&FileInfo);
                }
                else
                {
                    strcpy(_DirectoryShortName, &FileInfo.Name[_RootDirLen]);
                }
            }

            // if file is directory copy its name 
            if (FileInfo.Attr & 0x10)
            {
                strcpy(HeaderData->FileName, _DirectoryShortName);
            }
            // if file is file :)  build full name for that file
            else
            {
                strcpy(HeaderData->FileName, g_RxDesc.DirName);
                // get short file name
                char* pstr = strrchr(FileInfo.Name, '\\');
                if (pstr)
                    strcat(HeaderData->FileName, ++pstr);
                else
                    strcat(HeaderData->FileName, FileInfo.Name);
            }

            break;
        }

        return SUCCESS;
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
    Routine:     GetDirName
    ------------------------------------------------------------------------------
    Description:
        Extract short directory name from a full directory name. 
        Remember position of beginning of short dir name.

    Arguments:
        pFileInfo   - structure contained the name to parse

    Return Value:

    *****************************************************************************/

    void GetShortDirName(TFileInfo<TChar>& FileInfo)
    {
        if (_RootDirLen == 0xFFFF)
        {
            _RootDirLen = GetShortDirNameStartPos(FileInfo);
        }

        _pStringOperations->StrCpy(_DirectoryShortName, &pFileInfo->Name[_RootDirLen]);
    }

    int GetShortDirNameStartPos(TFileInfo<TChar>& FileInfo)
    {
        TChar* pstr;

        if (pFileInfo == NULL)
        {
            return;
        }

        // Check if dir name is full
        pstr = _pStringOperations->StrChr(pFileInfo->Name, ':');

        // If it is full, then extract the short dir name
        if (pstr)
        {
            // if file name is full then we need to cut root dir
            USHORT len = (USHORT)_pStringOperations->StrLen(pFileInfo->Name);
            pFileInfo->Name[len - 1] = 0;
            pstr = _pStringOperations->StrRChr(pFileInfo->Name, '\\');
            pFileInfo->Name[len - 1] = '\\';

            if (pstr)
            {
                return _pStringOperations->StrLen(pFileInfo->Name) - _pStringOperations->StrLen(++pstr);

            }
        }
        
        return 0;
    }

};



