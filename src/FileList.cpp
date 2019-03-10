#include "FileList.h"
#include "GlobalVars.h"

#include <regex>

using namespace std;

template <typename TFile, typename TChar>
FileList<TFile, TChar>::FileList(TChar* pAddList, TChar* pSourceFolder, IStringOperations<TChar>* pOps)
{
    if (pAddList == nullptr || pSourceFolder == nullptr || pOps == nullptr)
    {
        throw new std::invalid_argument("Null pointer is passed.");
    }

    _pOps = pOps;
    
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
                if (WildCardMatch(pShortFileName, g_ViewParam.WildCardMask))
                {
                    _List.insert(CreateFileInfo(pFileName);
                }
            }
        }

        // take pointer to the next file
        pFileName += _pOps->StrLen(pFileName) + 1;
    }

    // increase MaxLen if we will include full file name
    if (g_ViewParam.bFullName)
    {
        _FileNameColWidth += _pOps->StrLen(pSourceFolder);
    }

}


template <typename TFile, typename TChar>
decltype(auto) FileList<TFile, TChar>::CreateDirInfo(TChar* pFileName)
{
    WIN32_FILE_ATTRIBUTE_DATA FileDescription;

    auto pFileInfo = unique_ptr<TFile>(new TFile());

    pFileInfo->pOriginalName = pFileName;
    pFileInfo->pName = pFileName;

    bool rc = GetFileAttributesEx(
        pFileName,
        GetFileExInfoStandard,
        &FileDescription
    );

    pFileInfo->Attr = FileDescription.dwFileAttributes;
    pFileInfo->DateTime = FileDescription.ftLastWriteTime;
    pFileInfo->iSize = 0;
    pFileInfo->Attr  = 0;
    pFileInfo->pExt  = nullptr;
    pFileInfo->iType = FileType.TYPE_DIRECTORY;

    length = _pOps->StrLen(pFileInfo->pName) + CalculateIndent(pFileInfo->pOriginalName);
    if (length > _FileNameColWidth)
    {
        _FileNameColWidth = length;
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

template <typename TFile, typename TChar>
decltype(auto) FileList<TFile, TChar>::CreateFileInfo(TChar* pFileName)
{
    WIN32_FILE_ATTRIBUTE_DATA FileDescription;

    auto pFileInfo = unique_ptr<TFile>(new TFile());

    bool rc = GetFileAttributesEx(
        pFileName,
        GetFileExInfoStandard,
        &FileDescription
    );

    pFileInfo->Attr     = FileDescription.dwFileAttributes;
    pFileInfo->DateTime = FileDescription.ftLastWriteTime;
    pFileInfo->iType    = TYPE_FILE;

    if (rc)
    {
        pFileInfo->iSize = FileDescription.nFileSizeHigh;
        pFileInfo->iSize = pFileInfo->iSize << 32 | FileDescription.nFileSizeLow;
    }
    else
    {
        pFileInfo->iSize = 0;
    }


    pFileInfo->pOriginalName = pFileName;
    auto pShortFileName = GetShortFileName(pFileName);

    if (g_ViewParam.bFullName)
    {
        pFileInfo->pName = pFileName;
    }
    else
    {
        pFileInfo->pName = pShortFileName;
    }

    pFileInfo->pExt = _pOps->StrRChr(pShortFileName, '.');

    // if file name starts with '.' then think the file doesn't have extension
    if (pFileInfo->pExt == pShortFileName)
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
            _ExtensionColWidth = ExtLen;
        }
    }

    auto Length = (USHORT)strlen(pFileInfo->pName) + CalculateIndent(pFileInfo->pOriginalName);

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

template <typename TFile, typename TChar>
bool FileList<TFile, TChar>::IsDirectory(TChar* pFileName)
{
    return pFileName[_pOps->StrLen(pFileName) - 1] == "\\";
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

template <typename TFile, typename TChar>
USHORT FileList<TFile, TChar>::CalculateIndent(TChar* pStr)
{
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

template <typename TFile, typename TChar>
decltype(auto) FileList<TFile, TChar>::GetShortFileName(TChar* pFullName)
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

template <typename TFile, typename TChar>
bool FileList<TFile, TChar>::WildCardMatch(TChar* pFileName, string& pWildCard)
{
    basic_regex<TChar> RegExp(pWildCard);
    return regex_match(pFileName, RegExp;
}
