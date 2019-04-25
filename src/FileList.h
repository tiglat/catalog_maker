#pragma once

#include <string>
#include <set>
#include <regex>

#include "windows.h"

#include "GlobalVars.h"
#include "IStringOperations.h"

template <typename TChar>
struct FileListItem
{
    FileType    iType;
    DWORD64		iSize;
    FILETIME	DateTime;
    DWORD		Attr;

    // Pointer to path part of file name passed into the plugin by TotalCommander.
    // It could be full, partly full or short.
    TChar* pPath;

    // Short file name
    TChar* pName;

    // File name extension
    TChar* pExt;
};


template <typename TFile, typename TChar, typename TString> 
class FileList
{
public:
    FileList(TChar* pAddList, TChar* pSourceFolder, IStringOperations<TChar, TString>* pOps, std::basic_regex<TChar>& WildCardAsRegex) 
        : _pOps(pOps), _pSourceFolder(pSourceFolder)
    {
        if (pAddList == nullptr || pSourceFolder == nullptr || pOps == nullptr)
        {
            throw new std::invalid_argument("Null pointer is passed.");
        }

        // take pointer to the first file in the AddList
        TChar* pFileName = pAddList;

        _List = TListPtr(new TList(LessFileComparer(_pOps)));

        while (_pOps->IsNotNullOrEmpty(pFileName))
        {
            // calc length in advance because the file name will be splited in CreateFileInfo
            auto Len = _pOps->StrLen(pFileName) + 1;

            if (IsDirectory(pFileName))
            {
                if (g_ViewParam.bDirName)
                {
                    _List->insert(CreateDirInfo(pFileName));
                }
            }
            else
            {
                if (g_ViewParam.bFileName)
                {
                    auto pShortFileName = GetShortFileName(pFileName);

                    // check if the file name is matched to mask list
                    if (regex_match(pShortFileName, WildCardAsRegex))
                    {
                        _List->insert(CreateFileInfo(pFileName));
                    }
                }
            }

            // take pointer to the next file
            pFileName += Len;
        }

        // increase MaxLen if we will include full file name
        if (g_ViewParam.bFullName)
        {
            _FileNameColWidth += (USHORT) _pOps->StrLen(pSourceFolder);
        }

        First();
    }


    void First()
    {
        _Cursor = _List->begin();
    }

    void Next()
    {
        ++_Cursor;
    }

    bool IsEnd()
    {
        return _Cursor == _List->end();
    }

    TString GetCurrentElementLine()
    {
        TString result;
        auto pFileInfo = (*_Cursor);

        if (pFileInfo->iType == TYPE_DIRECTORY)
        {
            GetDirLine(result, pFileInfo);
        }
        else
        {
            GetFileLine(result, pFileInfo);
        }

        result += _pOps->GetEndLineChars();

        return result;
    }

private:

    /*****************************************************************************
        Routine:     operator()
    ------------------------------------------------------------------------------
        Description:
                    This function is used to sort file list and implements less operator
                    It compares two file descriptors

        Arguments:
                    lhs	- pointer to first descriptor
                    rhs	- pointer to second descriptor

        Return Value:
                    returns true if the first argument goes before the second argument
                    (first < second)

    *****************************************************************************/
//    template <typename TChar>
    class LessFileComparer
    {
    private:
        IStringOperations<TChar, TString>* _pOps;
    public:
        LessFileComparer(IStringOperations<TChar, TString>* ops) : _pOps(ops) {}

        bool operator() (const std::shared_ptr<TFile> &lhs, const std::shared_ptr<TFile> &rhs)
        {
            bool rv = false;

            // compare paths and then compare another attributes
            // depending on settings
            if (lhs->pPath && rhs->pPath)
            {
                auto result = _pOps->StrCaseInsensitiveCmp(lhs->pPath, rhs->pPath);

                if (result < 0)
                {
                    return GetResult(true);
                }

                if (result > 0)
                {
                    return GetResult(false);
                }
            }
            else
            {
                if (lhs->pPath == nullptr && rhs->pPath != nullptr)
                {
                    return GetResult(true);
                }

                if (lhs->pPath != nullptr && rhs->pPath == nullptr)
                {
                    return GetResult(false);
                }
            }

            // if paths are equal then we need to compare another attributes

            // if one of the items is directory than it is always less
            if (lhs->iType == FileType::FTYPE_DIRECTORY && rhs->iType == FileType::FTYPE_FILE)
            {
                return GetResult(true);
            }
            else if (rhs->iType == FileType::FTYPE_DIRECTORY && lhs->iType == FileType::FTYPE_FILE)
            {
                return GetResult(false);
            }

            // if user selected sort by extension
            if (g_SortParam.bExt)
            {
                if (lhs->pExt == nullptr)
                {
                    return GetResult(true);
                }

                if (rhs->pExt == nullptr)
                {
                    return GetResult(false);
                }

                auto result = _pOps->StrCaseInsensitiveCmp(lhs->pExt, rhs->pExt);

                if (result < 0)
                {
                    return GetResult(true);
                }

                if (result > 0)
                {
                    return GetResult(false);
                }

            }
            else if (g_SortParam.bSize) // if user selected sort by size
            {
                if (lhs->iSize < rhs->iSize)
                {
                    return GetResult(true);
                }
                else if (lhs->iSize > rhs->iSize)
                {
                    return GetResult(false);
                }
            }
            else if (g_SortParam.bDate) // if user selected sort by date
            {
                auto result = CompareFileTime(&lhs->DateTime, &rhs->DateTime);

                if (result < 0)
                {
                    return GetResult(true);
                }

                if (result > 0)
                {
                    return GetResult(false);
                }

            }
            else if (g_SortParam.bName) // if user selected sort by name
            {
                if (lhs->pName == nullptr)
                {
                    return GetResult(true);
                }

                if (rhs->pName == nullptr)
                {
                    return GetResult(false);
                }

                auto result = _pOps->StrCaseInsensitiveCmp(lhs->pName, rhs->pName);

                if (result < 0)
                {
                    return GetResult(true);
                }

                if (result > 0)
                {
                    return GetResult(false);
                }
            }

            return GetResult(false);
        }

    private:


        bool GetResult(bool value)
        {
            return g_SortParam.bDescent ? !value : value;
        }

    };

    // Proxy class for string operations
    IStringOperations<TChar, TString>* _pOps;
    TString _pSourceFolder;

    typedef std::set<std::shared_ptr<TFile>, LessFileComparer> TList;
    //typedef std::set<std::shared_ptr<TFile>, LessFileComparer>::interator TListIt;
    typedef std::shared_ptr<TList>    TListPtr;
    typedef std::shared_ptr<TFile> TFilePtr;

    TListPtr _List;
    typename std::set<std::shared_ptr<TFile>, LessFileComparer>::iterator  _Cursor;



    // Maximum length of file name column. Take into account indent length.
    USHORT _FileNameColWidth = 0;

    // Maximum length of file extension column. 
    USHORT _FileExtColWidth = 0;

    DWORD64	_TotalSize = 0;

    DWORD _TotalFiles = 0;

    USHORT _DirIndent = 0;
    USHORT _FileIndent = 0;



    bool IsDirectory(TChar* pFileName)
    {
        return pFileName[_pOps->StrLen(pFileName) - 1] == '\\';
    }

    void GetDirLine(TString& str, TFilePtr& pFileInfo)
    {
        if (g_ViewParam.bDirName)
        {
            if (g_FormatParam.bIndentFiles)
                _FileIndent = g_FormatParam.Width;

            if (g_FormatParam.bIndentAll)
            {
                _DirIndent = _pOps->StrNChr(pFileInfo->pPath, '\\') + 1;
                _DirIndent = (_DirIndent)* g_FormatParam.Width;
            }

            InsertIndent(str, _DirIndent);

            if (g_ViewParam.bApplyToDirs && g_ViewParam.bFullName)
            {
                // print full name including path
                str += _pSourceFolder;
            }

            str += pFileInfo->pPath;
            str += '\\';

            if (g_ViewParam.bApplyToDirs)
            {
                // make indent to get next column
                InsertIndent(str, _FileNameColWidth + 4 - str.length());

                if (g_ViewParam.bFileName && g_ViewParam.bExt && g_FormatParam.bExtSeparately)
                {
                    InsertIndent(str, _FileExtColWidth + 3);
                }

                // make empty indent, don't print size
                if (g_ViewParam.bFileName && g_ViewParam.bSize)
                {
                    InsertIndent(str, 18);
                }

                // print file date and time
                if (g_ViewParam.bDate || g_ViewParam.bTime)
                {
                    InsertDateTime(str, pFileInfo);
                }

                // print file attributes
                if (g_ViewParam.bAttr)
                {
                    InsertAttributeChars(str, pFileInfo);
                }
            }
        }

    }

    void GetFileLine(TString& str, TFilePtr& pFileInfo)
    {

        if (g_ViewParam.bFileName)
        {
            if (g_FormatParam.bIndentAll || g_FormatParam.bIndentFiles)
            {
                InsertIndent(str, _DirIndent + _FileIndent);
            }

            // print file path
            if (g_ViewParam.bFullName)
            {
                str += _pSourceFolder;
                if (pFileInfo->pPath != nullptr)
                {
                    str += pFileInfo->pPath;
                    str += '\\';
                }
            }

            // print file name
            if (pFileInfo->pName != nullptr)
            {
                str += pFileInfo->pName;
            }

            // print file extension with file name
            if (g_ViewParam.bExt && pFileInfo->pExt && !g_FormatParam.bExtSeparately)
            {
                str += '.';
                str += pFileInfo->pExt;
            }

            InsertIndent(str, _FileNameColWidth + 4 - str.length());

            // print file extension separately
            if (g_ViewParam.bExt && pFileInfo->pExt && g_FormatParam.bExtSeparately)
            {
                str += pFileInfo->pExt;
            }

            if (g_ViewParam.bExt && g_FormatParam.bExtSeparately)
            {
                InsertIndent(str, _FileExtColWidth);
            }

            // print file size
            if (g_ViewParam.bSize)
            {
                auto intStr = _pOps->ConvertIntToString(pFileInfo->iSize);
                auto len = intStr.length();
                InsertIndent(str, 15 - len);
                str += intStr;
                InsertIndent(str, 3);
            }

            // print file date and time
            if (g_ViewParam.bDate || g_ViewParam.bTime)
            {
                InsertDateTime(str, pFileInfo);
            }

            // print file attributes
            if (g_ViewParam.bAttr)
            {
                InsertAttributeChars(str, pFileInfo);
            }
        }
    }

    void InsertIndent(TString& str, USHORT num)
    {
        for (auto i = 0; i < num; i++)
        {
            str += ' ';
        }
    }

    void InsertAttributeChars(TString& str, TFilePtr& pFileItem)
    {
        if (pFileItem->Attr & FILE_ATTRIBUTE_READONLY)
        {
            //idx += sprintf( pBuf + idx, "r" );
            str += 'r';
        }
        else
        {
            //idx += sprintf(pBuf + idx, "-");
            str += '-';
        }

        if (pFileItem->Attr & FILE_ATTRIBUTE_ARCHIVE)
        {
            //idx += sprintf(pBuf + idx, "a");
            str += 'a';
        }
        else
        {
            //idx += sprintf(pBuf + idx, "-");
            str += '-';
        }

        if (pFileItem->Attr & FILE_ATTRIBUTE_HIDDEN)
        {
            //idx += sprintf(pBuf + idx, "h");
            str += 'h';
        }
        else
        {
            //idx += sprintf(pBuf + idx, "-");
            str += '-';
        }

        if (pFileItem->Attr & FILE_ATTRIBUTE_SYSTEM)
        {
            //idx += sprintf(pBuf + idx, "s");
            str += 's';
        }
        else
        {
            //idx += sprintf(pBuf + idx, "-");
            str += '-';
        }
    }

    void InsertDateTime(TString& str, TFilePtr& pFileItem)
    {
        SYSTEMTIME FileTime;

        FileTimeToLocalFileTime(&pFileItem->DateTime, &pFileItem->DateTime);
        FileTimeToSystemTime(&pFileItem->DateTime, &FileTime);

        if (g_ViewParam.bDate)
        {
            str += _pOps->ConvertDateToString(FileTime);
        }

        if (g_ViewParam.bTime)
        {
            str += _pOps->ConvertTimeToString(FileTime);
        }

    }

    /*****************************************************************************
        Routine:     CalculateIndent
    ------------------------------------------------------------------------------
        Description:
                    Calculate Indent before File name if it is necessary

        Arguments:
                    pStr	- pointer to string for analysis

        Return Value:
                    Indent length in chars


    *****************************************************************************/
    USHORT CalculateIndent(TChar* pStr)
    {
        if (pStr == nullptr)
        {
            return 0;
        }

        USHORT Indent = 0;

        if (g_FormatParam.bIndentAll)
        {
            Indent = _pOps->StrNChr(pStr, '\\');
            Indent = Indent * g_FormatParam.Width;
        }
        else
            if (g_FormatParam.bIndentFiles && g_ViewParam.bFileName)
            {
                Indent = _pOps->StrNChr(pStr, '\\');
                Indent = Indent ? g_FormatParam.Width : 0;
            }

        return Indent;
    }

    /*****************************************************************************
        Routine:     GetShortFileName
    ------------------------------------------------------------------------------
        Description:
                    Return short name of the given file
                    Trim file path

        Arguments:
                    pFullName	- pointer to full file name

        Return Value:
                    Pointer to first short name character in the
                    full file name

    *****************************************************************************/
    decltype(auto) GetShortFileName(TChar* pFullName)
    {
        TChar* pFileName = _pOps->StrRChr(pFullName, '\\');

        if (pFileName)
        {
            pFileName++;
        }
        else
        {
            pFileName = pFullName;
        }

        return pFileName;
    }


    decltype(auto) SplitFileName(TChar* pFullName, TChar** pPathName)
    {

        auto pShortName = GetShortFileName(pFullName);

        if (pShortName == pFullName)
        {
            *pPathName = nullptr;
        }
        else
        {
            *pPathName = pFullName;
            auto ptr = pShortName - 1;
            *ptr = 0;
        }

        return pShortName;
    }

    bool GetFileAttr(TChar* pFileName, LPVOID FileDescriptor);


    /*****************************************************************************
        Routine:     CreateDirInfo
    ------------------------------------------------------------------------------
        Description:
                    Gets meta information about the specified directory and creates
                    file descriptor object.

        Arguments:
                    pFileName - pointer to dir name

        Return Value:
                    File descriptor.


    *****************************************************************************/
    decltype(auto) CreateDirInfo(TChar* pFileName)
    {
        WIN32_FILE_ATTRIBUTE_DATA FileDescription = {0};
        auto pFileInfo = std::unique_ptr<TFile>(new TFile());

        pFileInfo->pPath = pFileName;
        pFileInfo->pName = nullptr;

        if ((g_ViewParam.bDate || g_ViewParam.bTime || g_ViewParam.bAttr) && g_ViewParam.bApplyToDirs)
        {
            GetFileAttr(pFileName, &FileDescription);
        }

        pFileInfo->Attr = FileDescription.dwFileAttributes;
        pFileInfo->DateTime = FileDescription.ftLastWriteTime;
        pFileInfo->iSize = 0;
        pFileInfo->Attr = 0;
        pFileInfo->pExt = nullptr;
        pFileInfo->iType = FileType::FTYPE_DIRECTORY;

        auto PathLen = _pOps->StrLen(pFileInfo->pPath);
        auto length = PathLen + CalculateIndent(pFileInfo->pPath);
        if (length > _FileNameColWidth)
        {
            _FileNameColWidth = (USHORT)length;
        }

        auto ptr = pFileInfo->pPath + PathLen - 1;
        *ptr = 0;

        return pFileInfo;
    }


    /*****************************************************************************
        Routine:     CreateFileInfo
    ------------------------------------------------------------------------------
        Description:
                    Gets meta information about the specified file and creates
                    file descriptor object.

        Arguments:
                    pFileName - pointer to file name

        Return Value:
                    File descriptor.


    *****************************************************************************/
    decltype(auto) CreateFileInfo(TChar* pFileName)
    {
        WIN32_FILE_ATTRIBUTE_DATA FileDescription = {0};

        auto pFileInfo = std::unique_ptr<TFile>(new TFile());

        bool rc = false;

        if (g_ViewParam.bDate || g_ViewParam.bTime || g_ViewParam.bAttr || g_ViewParam.bSize)
        {
            rc = GetFileAttr(pFileName, &FileDescription);
        }

        pFileInfo->Attr = FileDescription.dwFileAttributes;
        pFileInfo->DateTime = FileDescription.ftLastWriteTime;
        pFileInfo->iType = FileType::FTYPE_FILE;

        if (rc)
        {
            pFileInfo->iSize = FileDescription.nFileSizeHigh;
            pFileInfo->iSize = pFileInfo->iSize << 32 | FileDescription.nFileSizeLow;
        }
        else
        {
            pFileInfo->iSize = 0;
        }

        _TotalSize += pFileInfo->iSize;
        _TotalFiles++;

        auto Length = (USHORT)_pOps->StrLen(pFileName) + CalculateIndent(pFileName);

        pFileInfo->pName = SplitFileName(pFileName, &pFileInfo->pPath);

        pFileInfo->pExt = _pOps->StrRChr(pFileInfo->pName, '.');

        // if file name starts with '.' then think the file doesn't have extension
        if (pFileInfo->pExt == pFileInfo->pName)
        {
            pFileInfo->pExt = 0;
        }

        auto ExtLen = 0;
        if (pFileInfo->pExt)
        {
            *pFileInfo->pExt = 0;
            ++pFileInfo->pExt;
            ExtLen = _pOps->StrLen(pFileInfo->pExt);
            if (ExtLen > _FileExtColWidth)
            {
                _FileExtColWidth = (USHORT) ExtLen;
            }
        }

        if (g_ViewParam.bExt && !g_FormatParam.bExtSeparately)
        {
            Length += ExtLen + 1;
        }

        if (Length > _FileNameColWidth)
        {
            _FileNameColWidth = Length;
        }

        return pFileInfo;
    }


};




template <>
bool FileList<FileListItem<char>, char, std::string>::GetFileAttr(char* pFileName, LPVOID FileDescriptor)
{
    return GetFileAttributesExA(
        pFileName,
        GetFileExInfoStandard,
        FileDescriptor
    );
}


template <>
bool FileList<FileListItem<WCHAR>, WCHAR, std::wstring>::GetFileAttr(WCHAR* pFileName, LPVOID FileDescriptor)
{
    return GetFileAttributesExW(
        pFileName,
        GetFileExInfoStandard,
        FileDescriptor
    );
}


