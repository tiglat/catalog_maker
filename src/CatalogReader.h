#pragma once

#include <string>

#include "windows.h"

#include "GlobalVars.h"
#include "IStringOperations.h"

template <typename TChar, typename TString>
class CatalogReader
{
private: 
    // Buffer size in characters
    static const USHORT TEXT_LINE_LENGTH = 1000;

    // Max number of data columns in a file list
    static const USHORT LIST_COLUMN_NUMBER = 6;

    // Descriptor of catalog file
    HANDLE _CatalogFile;

    // Pointer to proxy class with functions for string manipulation.
    // It could be object of AnsiStringOperations or WideStringOperations class.
    IStringOperations<TChar, TString> *_pStringOperations;

    TChar		_pBuf[TEXT_LINE_LENGTH + 1] = { 0 };

    // Directory path without root dir.
    TChar		_DirectoryShortName[TEXT_LINE_LENGTH + 1] = { 0 };

    // Beginning position of short dir name. Short dir name is a part of path from root of file list.
    // For example, list file was created for files and subdirs of c:\somedir1\somedir2\somedir3.
    // c:\somedir1\somedir2 is a root directory for this list; somedir3 - short dir name
    USHORT		_ShortDirNamePos = 0xFFFF;

    // Position (in chars) in buffer that we have to read from (for parsing)
    USHORT		_iReadPos = 0;

    // Position (in chars) in buffer that we have to write to (from disk)
    USHORT		_iWritePos = 0;

    // Shows if we need to read from file more data
    bool		_bNeedData = true;

    // Shows if we read header lines or content
    UCHAR		_IsHeader = 2;

    // Number of read chars from catalog file
    DWORD		_ReturnedLength = 0;
    TFileInfo<TChar> _CurrentFileInfo = { 0 };

    using TColumnHandlerPtr = void (CatalogReader::*)(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo);
    
    // Describes layout of a column in a list
    struct TColumnInfo 
    {
        // start position of a column inside row
        USHORT              StartIdx;

        // width of a column (how many chars we should read from StartIdx)
        USHORT              Width;
    };

    // Contains layout of all columns in a list
    TColumnInfo _ListInfo[LIST_COLUMN_NUMBER] = {0};

    // Contains parsing functions for each column
    TColumnHandlerPtr _ColumnHandlers[LIST_COLUMN_NUMBER] = {nullptr};

public:
    
    CatalogReader(HANDLE CatalogFileHandle, IStringOperations<TChar, TString> *pOps) 
        : _CatalogFile(CatalogFileHandle), _pStringOperations(pOps)
    {
    }

    virtual ~CatalogReader()
    {}

    int ReadNext(TFileInfo<TChar>& FileInfo)
    {
        TString delimiters;
        TChar* token;
        int	rv;
        USHORT  len;

        memset(&FileInfo, 0, sizeof(FileInfo));
        delimiters += '\n';

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

                _bNeedData = false;
            }

            // get next string from read buffer
            token = _pStringOperations->StrTok(&_pBuf[_iReadPos], delimiters.c_str());

            // if there is no data for analyse 
            // then we should read next block
            if (token == nullptr)
            {
                _bNeedData = true;
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
                memcpy(_pBuf, token, len * sizeof(TChar));
                _iWritePos = len;
                _iReadPos = 0;
                _bNeedData = true;
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
                if (ParseListHeader(token) == false)
                {
                    return E_BAD_ARCHIVE;
                }
                continue;
            }
            else
            {
                ParseListContent(token, FileInfo);
            }

            _CurrentFileInfo = FileInfo;

            if (FileInfo.Attr & 0x10)
            {
                GetShortDirName(FileInfo);
            }

            break;
        }

        return SUCCESS;
    }

    /*****************************************************************************
        Routine:     GetFullFileName
    ------------------------------------------------------------------------------
        Description: Make full (beginning from file list root) file name 

        Arguments:
                FileInfo	 - [in] pointer to structure describing file in the string
                FullFileName - [out] pointer to string with full file name

        Return Value:
                None

    *****************************************************************************/

    void GetFullFileName(TFileInfo<TChar>& FileInfo, TChar* FullFileName, USHORT MaxFileNameLen = MAX_PATH)
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
            USHORT len = static_cast<USHORT>(_pStringOperations->StrLen(FullFileName));

            // get short file name
            TChar* pstr = _pStringOperations->StrRChr(FileInfo.Name, '\\');
            if (pstr)
            {
                _pStringOperations->StrNCat(FullFileName, ++pstr, MaxFileNameLen-len);
            }
            else
            {
                _pStringOperations->StrNCat(FullFileName, FileInfo.Name, MaxFileNameLen-len);
            }
        }
    }

    /*****************************************************************************
        Routine:     GetCurrentFileName
    ------------------------------------------------------------------------------
        Description: 
            Returns full file name which was read from list last time.

        Arguments:

        Return Value:
                Full file name of the last read item.

    *****************************************************************************/

    TChar* GetCurrentFileName()
    {
        return _CurrentFileInfo.Name;
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
        BOOL rv;

        rv = ReadFile(
            _CatalogFile,
            &_pBuf[_iWritePos],
            (TEXT_LINE_LENGTH - _iWritePos) * sizeof(TChar),
            &_ReturnedLength,
            NULL
        );

        if (rv &&  _ReturnedLength == 0)
        {
            return E_END_ARCHIVE;
        }

        if (rv == FALSE)
        {
            return E_EREAD;
        }

        if (sizeof(TChar) == sizeof(WCHAR))
        {
            _ReturnedLength = _ReturnedLength >> 1;
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
            _ShortDirNamePos = (USHORT) GetShortDirNameStartPos(FileInfo);
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

    bool ParseListHeader(TChar* pStr)
    {
        TChar* Offset;

        if (_IsHeader == 2) // first line of the header
        {
            //======= find and handle column "File name" ============
            Offset = _pStringOperations->StrStr(pStr, _pStringOperations->FILE_NAME_COLUMN);

            if (Offset == nullptr || Offset != pStr)
            {
                return false;
            }

            _ListInfo[COL_NAME].StartIdx = 1;
            _ColumnHandlers[COL_NAME] = &CatalogReader::ParseFileNameColumn;

            //======= find and handle column "Ext" ============
            Offset = _pStringOperations->StrStr(pStr, _pStringOperations->EXT_COLUMN);

            _ListInfo[COL_EXT].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ColumnHandlers[COL_EXT] = &CatalogReader::ParseExtColumn;

            _ListInfo[COL_NAME].Width =
                Offset ?
                (USHORT)(Offset - pStr) :
                0;

            //======= find and handle column "Size" ============
            Offset = _pStringOperations->StrStr(pStr, _pStringOperations->SIZE_COLUMN);

            _ListInfo[COL_SIZE].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ColumnHandlers[COL_SIZE] = &CatalogReader::ParseSizeColumn;

            _ListInfo[COL_SIZE].Width = 15;

            // if Size field exist and File name column width 
            // was not calculated yet
            if (Offset && _ListInfo[COL_NAME].Width == 0)
            {
                _ListInfo[COL_NAME].Width = (USHORT)(Offset - pStr);
            }

            // if Size field exist and Ext column width 
            // was not calculated yet
            if (Offset && _ListInfo[COL_EXT].Width == 0)
            {
                _ListInfo[COL_EXT].Width =
                    (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1);
            }

            //======= find and handle column "Date" ============
            Offset = _pStringOperations->StrStr(pStr, _pStringOperations->DATE_COLUMN);

            _ListInfo[COL_DATE].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ColumnHandlers[COL_DATE] = &CatalogReader::ParseDateColumn;

            _ListInfo[COL_DATE].Width = 10;

            if (Offset && _ListInfo[COL_NAME].Width == 0)
            {
                _ListInfo[COL_NAME].Width = (USHORT)(Offset - pStr);
            }

            if (Offset && _ListInfo[COL_EXT].Width == 0)
            {
                _ListInfo[COL_EXT].Width =
                    (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1);
            }

            //======= find and handle column "Time" ============
            Offset = _pStringOperations->StrStr(pStr, _pStringOperations->TIME_COLUMN);

            _ListInfo[COL_TIME].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ColumnHandlers[COL_TIME] = &CatalogReader::ParseTimeColumn;

            _ListInfo[COL_TIME].Width = 8;

            if (Offset && _ListInfo[COL_NAME].Width == 0)
            {
                _ListInfo[COL_NAME].Width = (USHORT)(Offset - pStr);
            }

            if (Offset && _ListInfo[COL_EXT].Width == 0)
            {
                _ListInfo[COL_EXT].Width =
                    (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1);
            }

            //======= find and handle column "Attr" ============
            Offset = _pStringOperations->StrStr(pStr, _pStringOperations->ATTR_COLUMN);

            _ListInfo[COL_ATTR].StartIdx =
                Offset ?
                (USHORT)(Offset - pStr + 1) :
                0;

            _ColumnHandlers[COL_ATTR] = &CatalogReader::ParseAttrColumn;

            _ListInfo[COL_ATTR].Width = 4;

            _ListInfo[COL_NAME].Width =
                _ListInfo[COL_NAME].Width ?
                _ListInfo[COL_NAME].Width :
                (Offset ? (USHORT)(Offset - pStr) : (USHORT)_pStringOperations->StrLen(pStr));

            _ListInfo[COL_EXT].Width =
                _ListInfo[COL_EXT].Width ?
                _ListInfo[COL_EXT].Width :
                (Offset ?
                (USHORT)(Offset - pStr - _ListInfo[COL_EXT].StartIdx - 1) :
                    (USHORT)_pStringOperations->StrLen(pStr) - _ListInfo[COL_EXT].StartIdx - 1);
        }

        if (_IsHeader == 1) // second and the last line of the header
        {
            // wrong file
            if (_ListInfo[COL_NAME].StartIdx == 0)
                return false;
        }

        _IsHeader--;

        return true;
    }

    /*****************************************************************************
        Routine:     ParseListContent
    ------------------------------------------------------------------------------
        Description: Analyse one item from list file by using header information

        Arguments:
                pStr	    -	pointer to string for analyse
                FileInfo	-	[out] pointer to structure describing file in the string

        Return Value:
                None

    *****************************************************************************/

    void ParseListContent(TChar* pStr, TFileInfo<TChar>& FileInfo)
    {
        // we don't know what colomns exist so we have to check all descriptors
        for (int i = 0; i < LIST_COLUMN_NUMBER; i++)
        {
            // if the column is present than run the handler function
            if (_ListInfo[i].StartIdx /*|| i == COL_NAME*/)
            {
                //(this->_ListInfo[i].*HandleFunc)(&pStr[_ListInfo[i].StartIdx - 1], _ListInfo[i].Width, FileInfo);
                (this->*_ColumnHandlers[i])(&pStr[_ListInfo[i].StartIdx - 1], _ListInfo[i].Width, FileInfo);
            }
        }

        return;
    }

    /*****************************************************************************
        Routine:     ParseFileNameColumn
    ------------------------------------------------------------------------------
        Description:
                    Extract file name from file list item

        Arguments:
                    pStr     - pointer to file list item
                    Width    - column width 
                    FileInfo - [out] pointer to structure describing file in the string
                    
        Return Value:

    *****************************************************************************/

    void ParseFileNameColumn(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo)
    {
        if (pStr == nullptr)
        {
            return;
        }

        int len = (int) _pStringOperations->StrLen(pStr);

        // if pStr is wrong or pStr is directory 
        // then we should correct StrLen

        // we can read wrong file which contains phrase "File name" in the first line and at first position 
        // In this case StrLen may be wrong

        // if pStr is directory we should correct length because dir name has 
        // longer length than column width It is correct situation

        if (Width < len && pStr[len - 2] != '\\')
        {
            len = Width;
        }

        if (len > PATH_LENGTH)
        {
            len = PATH_LENGTH;
        }

        DelSpacesAroundStr(FileInfo.Name, pStr, len);

        if (FileInfo.Name[_pStringOperations->StrLen(FileInfo.Name) - 1] == '\\')
        {
            FileInfo.Attr |= 0x10;
        }

    }

    /*****************************************************************************
        Routine:     ParseExtColumn
    ------------------------------------------------------------------------------
        Description:
                    Extract file extension from file list item

        Arguments:
                    pStr     - pointer to file list item
                    Width    - column width 
                    FileInfo - [out] pointer to structure describing file in the string

        Return Value:

    *****************************************************************************/

    void ParseExtColumn(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo)
    {
        TChar pExt[256];
        TString delimiters;

        delimiters += '.';

        if (pStr == nullptr)
        {
            return;
        }

        int len = (int) _pStringOperations->StrLen(pStr);

        if (Width < len)
        {
            len = Width;
        }

        DelSpacesAroundStr(pExt, pStr, len);

        // if file extension is not empty
        if (_pStringOperations->StrLen(pExt))
        {
            _pStringOperations->StrCat(FileInfo.Name, delimiters.c_str());
            _pStringOperations->StrCat(FileInfo.Name, pExt);
        }
    }

    /*****************************************************************************
        Routine:     ParseSizeColumn
    ------------------------------------------------------------------------------
        Description:
                    Extract file size from file list item

        Arguments:
                    pStr     - pointer to file list item
                    Width    - column width
                    FileInfo - [out] pointer to structure describing file in the string

        Return Value:

    *****************************************************************************/

    void ParseSizeColumn(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo)
    {
        TChar* token;
        TString Result;
        TChar Source[20];
        TString delimiters;

        delimiters += ',';

        if (pStr == nullptr)
        {
            return;
        }

        DelSpacesAroundStr(Source, pStr, Width);

        if (!IsFileSize(Source))
        {
            return;
        }

        token = _pStringOperations->StrTok(Source, delimiters.c_str());

        while (token != nullptr)
        {
            Result += token;
            token = _pStringOperations->StrTok(NULL, delimiters.c_str());
        }

        FileInfo.iSize = _pStringOperations->ConvertStringToInt(Result);
        return;
    }

    /*****************************************************************************
    Routine:     ParseDateColumn
    ------------------------------------------------------------------------------
        Description:
                    Extract file date from file list item

        Arguments:
                    pStr     - pointer to file list item
                    Width    - column width
                    FileInfo - [out] pointer to structure describing file in the string

        Return Value:

    *****************************************************************************/
    
    void ParseDateColumn(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo)
    {
        TChar* token;
        TChar Source[20];
        TString delimiters;

        delimiters += '.';

        if (pStr == nullptr)
        {
            return;
        }

        memcpy(Source, pStr, Width * sizeof(TChar));
        Source[Width] = 0;

        if (!IsFileDate(Source))
        {
            return;
        }

        token = _pStringOperations->StrTok(Source, delimiters.c_str());

        if (token)
        {
            FileInfo.Day = (DWORD) _pStringOperations->ConvertStringToInt(token);
            token = _pStringOperations->StrTok(NULL, delimiters.c_str());
        }

        if (token)
        {
            FileInfo.Month = (DWORD)_pStringOperations->ConvertStringToInt(token) - 1;
            token = _pStringOperations->StrTok(NULL, delimiters.c_str());
        }

        if (token)
        {
            FileInfo.Year = (DWORD)_pStringOperations->ConvertStringToInt(token) - 1900;
        }

        return;
    }

    /*****************************************************************************
    Routine:     ParseTimeColumn
    ------------------------------------------------------------------------------
        Description:
                    Extract file time from file list item

        Arguments:
                    pStr     - pointer to file list item
                    Width    - column width
                    FileInfo - [out] pointer to structure describing file in the string

        Return Value:

    *****************************************************************************/
    
    void ParseTimeColumn(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo)
    {
        TChar* token;
        TChar Source[20];
        TString delimiters;

        delimiters += ':';
        delimiters += '.';

        if (pStr == nullptr)
            return;

        memcpy(Source, pStr, Width * sizeof(TChar));
        Source[Width] = 0;

        if (!IsFileTime(Source))
            return;

        token = _pStringOperations->StrTok(Source, delimiters.c_str());

        if (token)
        {
            FileInfo.Hour = (DWORD) _pStringOperations->ConvertStringToInt(token);
            token = _pStringOperations->StrTok(NULL, delimiters.c_str());
        }

        if (token)
        {
            FileInfo.Minute = (DWORD) _pStringOperations->ConvertStringToInt(token);
            token = _pStringOperations->StrTok(NULL, delimiters.c_str());
        }

        if (token)
        {
            FileInfo.Second = (DWORD) _pStringOperations->ConvertStringToInt(token);
        }

        return;
    }

    /*****************************************************************************
    Routine:     ParseAttrColumn
    ------------------------------------------------------------------------------
        Description:
                    Extract file attributes from file list item

        Arguments:
                    pStr     - pointer to file list item
                    Width    - column width
                    FileInfo - [out] pointer to structure describing file in the string

        Return Value:

    *****************************************************************************/
    
    void ParseAttrColumn(TChar* pStr, USHORT Width, TFileInfo<TChar>& FileInfo)
    {
        DWORD Result = 0;
        TChar Source[20];

        if (pStr == nullptr)
        {
            return;
        }

        // get value of attributes from the string
        memcpy(Source, pStr, Width * sizeof(TChar));
        Source[Width] = 0;

        // test if the substring is attributes
        if (!IsFileAttr(Source))
        {
            return;
        }

        // analyse attr substring
        for (int i = 0; i < 4; i++)
        {
            switch (pStr[i])
            {
            case 'a':
                Result |= 0x20;
                break;

            case 'h':
                Result |= 0x02;
                break;

            case 'r':
                Result |= 0x01;
                break;

            case 's':
                Result |= 0x04;
                break;
            }
        }

        FileInfo.Attr |= Result;
        return;
    }

    /*****************************************************************************
        Routine:     DelSpacesAroundStr
    ------------------------------------------------------------------------------
        Description:
                    Removes spaces in the beginning and end of string

        Arguments:
                    pStr - pointer to string

        Return Value:

    *****************************************************************************/

    void DelSpacesAroundStr(TChar* pResultStr, TChar* pStr, USHORT Len)
    {
        USHORT i = Len;
        USHORT StrLen;

        // count postfix spaces
        while ((pStr[i - 1] == 32 || pStr[i - 1] == 0x0D) && i > 0)
        {
            i--;
        }

        StrLen = i;
        i = 0;

        // count prefix spaces
        while (pStr[i] == 32 && i < StrLen)
        {
            i++;
        }

        StrLen -= i;

        // get substring between spaces
        memcpy(pResultStr, &pStr[i], StrLen * sizeof(TChar));
        pResultStr[StrLen] = 0;
    }

    /*****************************************************************************
        Routine:     IsFileDate
    ------------------------------------------------------------------------------
        Description:
                    Determines if pStr is file date

        Arguments:
                    pStr	- pointer to string

        Return Value:

    *****************************************************************************/

    bool IsFileDate(TChar* pStr)
    {
        if (pStr == nullptr)
        {
            return false;
        }

        //if ( !strchr( pStr, '/' ) )	
        //	Result = FALSE;

        if (!iswdigit(pStr[0])
            || !iswdigit(pStr[1])
            || !iswdigit(pStr[3])
            || !iswdigit(pStr[4])
            || !iswdigit(pStr[6])
            || !iswdigit(pStr[7])
            || !(pStr[2] == '.')
            || !(pStr[5] == '.'))
        {
            return false;
        }

        return true;
    }

    /*****************************************************************************
        Routine:     IsFileTime
    ------------------------------------------------------------------------------
        Description:
                    Determines if pStr is file time

        Arguments:
                    pStr	- pointer to string

        Return Value:

    *****************************************************************************/

    bool IsFileTime(TChar* pStr)
    {
        if (pStr == nullptr)
        {
            return false;
        }

        if (_pStringOperations->StrChr(pStr, ':') && !_pStringOperations->StrChr(pStr, '\\'))
        {
            return true;
        }

        return false;
    }

    /*****************************************************************************
        Routine:     IsFileAttr
    ------------------------------------------------------------------------------
        Description:
                    Determines if pStr is file attributes

        Arguments:
                    pStr	- pointer to string

        Return Value:

    *****************************************************************************/

    bool IsFileAttr(TChar* pStr)
    {
        if (pStr == nullptr)
        {
            return false;
        }

        if (_pStringOperations->StrLen(pStr) == 4)
        {
            for (int i = 0; i < 4; i++)
                if (pStr[i] != 'r' &&
                    pStr[i] != 'a' &&
                    pStr[i] != 'h' &&
                    pStr[i] != 's' &&
                    pStr[i] != '-'
                    )
                {
                    return false;
                }
        }
        else
        {
            return false;
        }
        
        return true;
    }

    /*****************************************************************************
        Routine:     IsFileSize
    ------------------------------------------------------------------------------
        Description:
                    Determines if pStr is file size

        Arguments:
                    pStr	- pointer to string

        Return Value:

    *****************************************************************************/

    bool IsFileSize(TChar *pStr)
    {
        UINT Len = (UINT)_pStringOperations->StrLen(pStr);
        UINT i;

        if (pStr == nullptr)
        {
            return false;
        }

        for (i = 0; i < Len; i++)
        {
            if (!(iswdigit(pStr[i]) || pStr[i] == ','))
            {
                return false;
            }
        }

        return true;
    }

};





