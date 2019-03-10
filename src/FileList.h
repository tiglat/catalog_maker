#pragma once

#include <string>

#include "windows.h"

#include "GlobalVars.h"
#include "IStringOperations.h"

struct FileInfoBase
{
    FileType    iType;
    DWORD64		iSize;
    FILETIME	DateTime;
    DWORD		Attr;
};

// ANSI version
struct AnsiFileInfo : FileInfoBase
{
    // File name passed into the plugin by TotalCommander.
    // It could be full, partly full or short.
    char* pOriginalName; 

    // Short file name
    char* pName;

    // File name extension
    char* pExt;
};


// Unicode version
struct WideFileInfo : FileInfoBase
{
    // File name passed into the plugin by TotalCommander.
    // It could be full, partly full or short.
    WCHAR* pOriginalName;

    // Short file name
    WCHAR* pName;

    // File name extension
    WCHAR* pExt;
};

template <typename TFile> class FileComparer
{
    bool operator() (std::shared_ptr<TFile> &lhs, std::shared_ptr<TFile> &rhs)
    {

    }
};


template <typename TFile, typename TChar> class FileList
{
public:
    FileList(TChar* pAddList, TChar* pSourceFolder, IStringOperations<TChar>* pOps);


private:
    std::set<std::shared_ptr<TFile>, FileComparer<TFile>> _List;

    // Proxy class for string operations
    IStringOperations<TChar>* _pOps;

    // Maximum length of file name column. Take into account indent length.
    USHORT _FileNameColWidth;

    // Maximum length of file extension column. 
    USHORT _ExtensionColWidth;

    bool IsDirectory(TChar* pFileName);

    USHORT CalculateIndent(TChar* pStr);

    decltype(auto) GetShortFileName(TChar* pFullName);

    bool WildCardMatch(TChar* pFileName, std::string& pWildCard);
    
    decltype(auto) CreateDirInfo(TChar* pFileName);
    decltype(auto) CreateFileInfo(TChar* pFileName);
};