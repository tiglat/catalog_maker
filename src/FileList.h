#pragma once

#include <string>
#include <set>
#include <regex>

#include "windows.h"

#include "GlobalVars.h"
#include "IStringOperations.h"

template <typename TChar>
struct FileInfoBase
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

////ANSI version
//struct AnsiFileInfo : FileInfoBase
//{
//    // Pointer to path part of file name passed into the plugin by TotalCommander.
//    // It could be full, partly full or short.
//    char* pPath;
//
//    // Short file name
//    char* pName;
//
//    // File name extension
//    char* pExt;
//};
//
//
//// Unicode version
//struct WideFileInfo : FileInfoBase
//{
//    // Pointer to path part of file name passed into the plugin by TotalCommander.
//    // It could be full, partly full or short.
//    WCHAR* pPath;
//
//    // Short file name
//    WCHAR* pName;
//
//    // File name extension
//    WCHAR* pExt;
//};



template <typename TFile, typename TChar> 
class FileList
{
public:
    FileList(TChar* pAddList, TChar* pSourceFolder, IStringOperations<TChar>* pOps)
    {
        if (pAddList == nullptr || pSourceFolder == nullptr || pOps == nullptr)
        {
            throw new std::invalid_argument("Null pointer is passed.");
        }

        SetCurrentDirectory(pSourceFolder);

        _pOps = pOps;
        std::basic_regex<TChar> WildCardAsRegex(g_WildCardPattern);

        // take pointer to the first file in the AddList
        TChar* pFileName = pAddList;

        while (_pOps->IsNotNullOrEmpty(pFileName))
        {
            if (IsDirectory(pFileName))
            {
                if (g_ViewParam.bDirName)
                {
                    _List.insert(CreateDirInfo(pFileName));
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
                        _List.insert(CreateFileInfo(pFileName));
                    }
                }
            }

            // take pointer to the next file
            pFileName += _pOps->StrLen(pFileName) + 1;
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
    //class LessFileComparer
    //{
    //    bool operator() (std::shared_ptr<TFile> &lhs, std::shared_ptr<TFile> &rhs);
    //};


    std::set<std::shared_ptr<TFile>> _List;

    // Proxy class for string operations
    IStringOperations<TChar>* _pOps;

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

    decltype(auto) CreateDirInfo(TChar* pFileName)
    {
        WIN32_FILE_ATTRIBUTE_DATA FileDescription;

        auto pFileInfo = std::unique_ptr<TFile>(new TFile());

        pFileInfo->pPath = pFileName;
        pFileInfo->pName = nullptr;

        bool rc = GetFileAttributesEx(
            pFileName,
            GetFileExInfoStandard,
            &FileDescription
        );

        pFileInfo->Attr = FileDescription.dwFileAttributes;
        pFileInfo->DateTime = FileDescription.ftLastWriteTime;
        pFileInfo->iSize = 0;
        pFileInfo->Attr = 0;
        pFileInfo->pExt = nullptr;
        pFileInfo->iType = FileType::FTYPE_DIRECTORY;

        auto length = _pOps->StrLen(pFileInfo->pPath) + CalculateIndent(pFileInfo->pPath);
        if (length > _FileNameColWidth)
        {
            _FileNameColWidth = (USHORT) length;
        }

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
        WIN32_FILE_ATTRIBUTE_DATA FileDescription;

        auto pFileInfo = std::unique_ptr<TFile>(new TFile());

        bool rc = GetFileAttributesEx(
            pFileName,
            GetFileExInfoStandard,
            &FileDescription
        );

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

        auto Length = (USHORT)strlen(pFileInfo->pName) + CalculateIndent(pFileInfo->pPath);

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

*****************************************************************************/

//template <typename TFile, typename TChar>
//bool FileList<TFile, TChar>::LessFileComparer::operator() (std::shared_ptr<TFile> &lhs, std::shared_ptr<TFile> &rhs)
//{
//    bool rv;
//
//    // compare paths and then compare another attributes
//    // depending on settings
//    if (lhs->pPath && rhs->pPath)
//    {
//        rv = _pOps->StrCaseInsensitiveCmp(lhs->pPath, rhs->pPath);
//    }
//    else
//    {
//        if (lhs->pPath == 0 && rhs->pPath == 0)
//        {
//            rv = 0;
//        }
//        else
//        {
//            if (lhs->pPath == 0)
//                rv = -1;
//            else
//                rv = 1;
//        }
//    }
//
//    // if paths are equal then we need to compare another attributes
//    if (rv == 0)
//    {
//        // if one of the items is directory than it is always less
//        if (lhs->pName == 0)
//        {
//            rv = -1;
//            return (rv >= 0 ? false : true);
//        }
//        else
//            if (rhs->pName == 0)
//            {
//                rv = 1;
//                return (rv >= 0 ? false : true);
//            }
//
//        // if user selected sort by extension
//        if (g_SortParam.bExt)
//        {
//            if (lhs->pExt == 0)
//                rv = -1;
//
//            if (rhs->pExt == 0)
//                rv = 1;
//
//            if (lhs->pExt && rhs->pExt)
//                rv = _stricmp(lhs->pExt, rhs->pExt);
//        }
//
//        // if user selected sort by size
//        if (g_SortParam.bSize)
//        {
//            if (lhs->iSize < rhs->iSize)
//            {
//                rv = -1;
//            }
//            else
//            {
//                if (lhs->iSize > rhs->iSize)
//                    rv = 1;
//                else
//                    rv = 0;
//            }
//        }
//
//        // if user selected sort by date
//        if (g_SortParam.bDate)
//        {
//            rv = CompareFileTime(
//                &lhs->DateTime,
//                &rhs->DateTime
//            );
//        }
//
//        // if user selected sort by name
//        if (g_SortParam.bName || rv == 0)
//        {
//            if (lhs->pName == 0)
//                rv = -1;
//
//            if (rhs->pName == 0)
//                rv = 1;
//
//            if (lhs->pName && rhs->pName)
//                //rv = _wcsicmp( (wchar_t*)lhs->pName, (wchar_t*)rhs->pName );
//                rv = _stricmp(lhs->pName, rhs->pName);
//        }
//
//        if (g_SortParam.bDescent)
//        {
//            rv *= -1;
//        }
//
//    } // if rv
//
//    return (rv >= 0 ? false : true);
//
//}
//
