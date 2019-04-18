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


template <typename TFile, typename TChar> 
class FileList
{
public:
    FileList(TChar* pAddList, TChar* pSourceFolder, IStringOperations<TChar>* pOps, std::basic_regex<TChar>& WildCardAsRegex)
    {
        if (pAddList == nullptr || pSourceFolder == nullptr || pOps == nullptr)
        {
            throw new std::invalid_argument("Null pointer is passed.");
        }

        _pOps = pOps;

        // take pointer to the first file in the AddList
        TChar* pFileName = pAddList;

        _List = TListPtr(new std::set<std::shared_ptr<TFile>, LessFileComparer>(LessFileComparer(_pOps)));

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

    }

    ~FileList()
    {}


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
        IStringOperations<TChar>* _pOps;
    public:
        LessFileComparer(IStringOperations<TChar>* ops) : _pOps(ops) {}

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
    IStringOperations<TChar>* _pOps;

    typedef std::shared_ptr<std::set<std::shared_ptr<TFile>, LessFileComparer>>    TListPtr;

    TListPtr _List;

    // Maximum length of file name column. Take into account indent length.
    USHORT _FileNameColWidth = 0;

    // Maximum length of file extension column. 
    USHORT _ExtensionColWidth = 0;


    bool IsDirectory(TChar* pFileName)
    {
        return pFileName[_pOps->StrLen(pFileName) - 1] == '\\';
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
            ++pFileInfo->pExt;
            auto ExtLen = _pOps->StrLen(pFileInfo->pExt);
            if (ExtLen > _ExtensionColWidth)
            {
                _ExtensionColWidth = (USHORT) ExtLen;
            }
        }

        auto Length = (USHORT)_pOps->StrLen(pFileInfo->pName) + CalculateIndent(pFileInfo->pPath);

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
bool FileList<FileListItem<char>, char>::GetFileAttr(char* pFileName, LPVOID FileDescriptor)
{
    return GetFileAttributesExA(
        pFileName,
        GetFileExInfoStandard,
        FileDescriptor
    );
}


template <>
bool FileList<FileListItem<WCHAR>, WCHAR>::GetFileAttr(WCHAR* pFileName, LPVOID FileDescriptor)
{
    return GetFileAttributesExW(
        pFileName,
        GetFileExInfoStandard,
        FileDescriptor
    );
}


