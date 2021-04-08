#pragma once

#include <string>
#include <list>
#include <regex>
//#include <filesystem>
//#include <chrono> 

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
    TChar *pPath;

    // Short file name
    TChar *pName;

    // File name extension
    TChar *pExt;

};


template <typename TFile, typename TChar, typename TString> 
class FileList
{
private:
    // Proxy class for string operations
    IStringOperations<TChar, TString> *_pStringOperations;
    TString _pSourceFolder;

    typedef std::shared_ptr<TFile> TFilePtr;
    typedef std::list<TFilePtr> TList;
    typedef std::shared_ptr<TList> TListPtr;

    TListPtr _List;
    typename std::list<TFilePtr>::iterator  _Cursor;



    // Maximum length of file name column. Take into account indent length.
    USHORT _FileNameColWidth = 0;

    // Maximum length of file extension column. 
    USHORT _FileExtColWidth = 0;

    DWORD64	_TotalSize = 0;

    DWORD _TotalFiles = 0;

    USHORT _DirIndent = 0;

public:
    FileList(TChar *pAddList, TChar *pSourceFolder, IStringOperations<TChar, TString> *pOps, std::basic_regex<TChar> &WildCardAsRegex) 
        : _pStringOperations(pOps), _pSourceFolder(pSourceFolder)
    {
        if (pAddList == nullptr || pSourceFolder == nullptr || pOps == nullptr)
        {
            throw std::invalid_argument("Null pointer is passed.");
        }

        // take pointer to the first file in the AddList
        TChar *pFileName = pAddList;

        _List = TListPtr(new TList());

        while (_pStringOperations->IsNotNullOrEmpty(pFileName))
        {
            // calc length in advance because the file name will be splited in CreateFileInfo
            auto Len = _pStringOperations->StrLen(pFileName) + 1;

            if (IsDirectory(pFileName))
            {
                if (g_ViewParam.bDirName)
                {
                    _List->push_back(CreateDirInfo(pFileName));
                }
            }
            else
            {
                if (g_ViewParam.bFileName || g_ViewParam.bDirSize)
                {
                    auto pShortFileName = GetShortFileName(pFileName);

                    // check if the file name is matched to mask list
                    if (std::regex_match(pShortFileName, WildCardAsRegex))
                    {
                        _List->push_back(CreateFileInfo(pFileName));
                    }
                }
            }

            // take pointer to the next file
            pFileName += Len;
        }

        // increase MaxLen if we will include full file name
        if (g_ViewParam.bFullName)
        {
            _FileNameColWidth += (USHORT) _pStringOperations->StrLen(pSourceFolder);
        }

        // if list should contain directories and dir size is selected
        if (g_ViewParam.bDirName && g_ViewParam.bDirSize)
        {
            First();
            CalcDirectorySize(nullptr);

            // we need to remove files from list because they were added to calculate dir size only 
            if (!g_ViewParam.bFileName)
            {
                _List->remove_if([](const TFilePtr& value) {return value->iType == FileType::FTYPE_FILE; });
            }
        }

        //auto start = std::chrono::high_resolution_clock::now();
        if (!g_SortParam.bUnsorted)
        {
            _List->sort(LessFileComparer(_pStringOperations));
        }
        //auto stop = std::chrono::high_resolution_clock::now();
        //auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

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

    TChar* GetCurrentFileName()
    {
        return (*_Cursor)->pName;
    }

    DWORD64 GetCurrentFileSize()
    {
        return (*_Cursor)->iSize;
    }

    TString GetCurrentFileLine()
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

        result += _pStringOperations->GetEndLineChars();

        return result;
    }

    TString GetHeaderLine()
    {
        TString result;

        result += _pStringOperations->FILE_NAME_COLUMN;

        InsertIndent(result, _FileNameColWidth + 4 - result.length());

        if (g_ViewParam.bFileName)
        {
            if (g_ViewParam.bExt && g_FormatParam.bExtSeparately)
            {
                result += _pStringOperations->EXT_COLUMN;
                InsertIndent(result, _FileExtColWidth);
            }

            if (g_ViewParam.bSize)
            {
                result += _pStringOperations->SIZE_COLUMN;
                InsertIndent(result, 14);
            }

            if (g_ViewParam.bDate)
            {
                result += _pStringOperations->DATE_COLUMN;
                InsertIndent(result, 8);
            }

            if (g_ViewParam.bTime)
            {
                result += _pStringOperations->TIME_COLUMN;
                InsertIndent(result, 6);
            }

            if (g_ViewParam.bAttr)
            {
                result += _pStringOperations->ATTR_COLUMN;
            }
        }
        else if (g_ViewParam.bDirName && g_ViewParam.bApplyToDirs)
        {
            if (g_ViewParam.bDirSize)
            {
                result += _pStringOperations->SIZE_COLUMN;
                InsertIndent(result, 14);
            }

            if (g_ViewParam.bDate)
            {
                result += _pStringOperations->DATE_COLUMN;
                InsertIndent(result, 8);
            }

            if (g_ViewParam.bTime)
            {
                result += _pStringOperations->TIME_COLUMN;
                InsertIndent(result, 6);
            }

            if (g_ViewParam.bAttr)
            {
                result += _pStringOperations->ATTR_COLUMN;
            }
        }

        result += _pStringOperations->GetEndLineChars();
        return result;
    }


    TString GetDividingLine()
    {
        TString result;

        InsertChar(result, '-', _FileNameColWidth);
        InsertIndent(result, 4);

        if ( g_ViewParam.bFileName )
        {
            if ( g_ViewParam.bExt && g_FormatParam.bExtSeparately )
            {
                InsertChar(result, '-', _FileExtColWidth);
                InsertIndent(result, 3);
            }

            if (g_ViewParam.bSize)
            {
                InsertChar(result, '-', 15);
                InsertIndent(result, 3);
            }

            if (g_ViewParam.bDate)
            {
                InsertChar(result, '-', 10);
                InsertIndent(result, 2);
            }

            if ( g_ViewParam.bTime )
            {
                InsertChar(result, '-', 8);
                InsertIndent(result, 2);
            }

            if ( g_ViewParam.bAttr )
            {
                InsertChar(result, '-', 4);
            }
            
        }
        else if ( g_ViewParam.bDirName && g_ViewParam.bApplyToDirs )
        {
            if (g_ViewParam.bDirSize)
            {
                InsertChar(result, '-', 15);
                InsertIndent(result, 3);
            }

            if (g_ViewParam.bDate)
            {
                InsertChar(result, '-', 10);
                InsertIndent(result, 2);
            }

            if (g_ViewParam.bTime)
            {
                InsertChar(result, '-', 8);
                InsertIndent(result, 2);
            }

            if (g_ViewParam.bAttr)
            {
                InsertChar(result, '-', 4);
            }
        }

        result += _pStringOperations->GetEndLineChars();
        return result;
    }

    TString GetFooterLine()
    {
        TString result;

        // print total size and number of the files
        if (g_ViewParam.bFileName)
        {
            result += _pStringOperations->FOOTER_TOTAL_FILES;
            result += _pStringOperations->ConvertIntToString(_TotalFiles);

            if (g_ViewParam.bSize)
            {
                result += _pStringOperations->FOOTER_TOTAL_SIZE;
                result += _pStringOperations->ConvertFileSizeToString(_TotalSize);
            }

            // don't include symbol '\r' in order to read 
            // the list file easier
            result += '\n';
        }

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
        IStringOperations<TChar, TString>* _pStringOperations;

        int CompareFilePaths(TChar *lhs, TChar *rhs)
        {
            if (lhs == nullptr && rhs != nullptr)
            {
                return -1;
            }

            if (lhs != nullptr && rhs == nullptr)
            {
                return 1;
            }

            if (lhs == nullptr && rhs == nullptr)
            {
                return 0;
            }

            TChar *lhsCh = lhs;
            TChar *rhsCh = rhs;
            TChar *lhsSubDir = lhs;
            TChar *rhsSubDir = rhs;
            int result = 0;

            TChar lhsCachedChar = 0;
            TChar rhsCachedChar = 0;

            size_t lhsLen = _pStringOperations->StrLen(lhs);
            size_t rhsLen = _pStringOperations->StrLen(rhs);

            // In the beginning to avoid redundant copy operation I split full file name on two strings, path and file name, 
            // by inserting zero instead slash.
            // Now we want to paths have slash at the end.
            // So we need to replace terminating zero in path with slash and next symbol (first char of file name) with zero.
            if (lhs[lhsLen - 1] != '\\')
            {
                lhsCachedChar = lhs[lhsLen + 1];
                lhs[lhsLen] = '\\';
                lhs[lhsLen + 1] = 0;
            }

            if (rhs[rhsLen - 1] != '\\')
            {
                rhsCachedChar = rhs[rhsLen + 1];
                rhs[rhsLen] = '\\';
                rhs[rhsLen + 1] = 0;
            }

            // make comparision until subdirs are equal and it is not the end of path
            while (result == 0 && *lhsCh != 0 && *rhsCh != 0)
            {
                // find end of subdir
                while (*lhsCh != '\\' && *lhsCh != 0)
                {
                    ++lhsCh;
                };

                while (*rhsCh != '\\' && *rhsCh != 0)
                {
                    ++rhsCh;
                };

                // replace slash with terminating zero to make comparision of one level subdirs only
                *lhsCh = 0;
                *rhsCh = 0;

                result = _pStringOperations->StrCaseInsensitiveCmp(lhsSubDir, rhsSubDir);

                // restore slashes
                *lhsCh = '\\';
                *rhsCh = '\\';

                // go to next subdir
                lhsSubDir = ++lhsCh;
                rhsSubDir = ++rhsCh;
            }

            // check why we exited from cycle above
            if (*lhsCh == 0 && *rhsCh != 0 && result == 0)
            {
                result = -1;
            }

            if (*lhsCh != 0 && *rhsCh == 0 && result == 0)
            {
                result = 1;
            }

            // Restore file name 
            if (lhsCachedChar != 0)
            {
                lhs[lhsLen + 1] = lhsCachedChar;
                lhs[lhsLen] = 0;
            }

            if (rhsCachedChar != 0)
            {
                rhs[rhsLen + 1] = rhsCachedChar;
                rhs[rhsLen] = 0;
            }

            return result;
        }

    public:
        LessFileComparer(IStringOperations<TChar, TString>* ops) : _pStringOperations(ops) {}

        bool operator() (const std::shared_ptr<TFile> &lhs, const std::shared_ptr<TFile> &rhs)
        {
            bool rv = false;

            // compare paths and then compare another attributes
            // depending on settings
            if (lhs->pPath && rhs->pPath)
            {
                // commented code works 7 times slower than CompareFilePaths method

                //std::filesystem::path lhsPath = lhs->pPath;
                //std::filesystem::path rhsPath = rhs->pPath;

                //if (lhs->iType == FileType::FTYPE_FILE)
                //{
                //    lhsPath += "\\";
                //}

                //if (rhs->iType == FileType::FTYPE_FILE)
                //{
                //    rhsPath += "\\";
                //}

                //int rc = lhsPath.compare(rhsPath);

                int rc = CompareFilePaths(lhs->pPath, rhs->pPath);

                if (rc != 0)
                {
                    return rc < 0;
                }
            }
            else
            {
                if (lhs->pPath == nullptr && rhs->pPath != nullptr)
                {
                    return true;
                }

                if (lhs->pPath != nullptr && rhs->pPath == nullptr)
                {
                    return false;
                }
            }

            // if paths are equal then we need to compare another attributes

            // if one of the items is directory than it is always less
            if (lhs->iType == FileType::FTYPE_DIRECTORY && rhs->iType == FileType::FTYPE_FILE)
            {
                return true;
            }
            else if (rhs->iType == FileType::FTYPE_DIRECTORY && lhs->iType == FileType::FTYPE_FILE)
            {
                return false;
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

                auto result = _pStringOperations->StrCaseInsensitiveCmp(lhs->pExt, rhs->pExt);

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

                auto result = _pStringOperations->StrCaseInsensitiveCmp(lhs->pName, rhs->pName);

                if (result < 0)
                {
                    return GetResult(true);
                }

                if (result > 0)
                {
                    return GetResult(false);
                }
            }

            return false;
        }

    private:


        bool GetResult(bool value)
        {
            return g_SortParam.bDescent ? !value : value;
        }

    };


    DWORD64 CalcDirectorySize(TFilePtr dir)
    {
        TFilePtr dirInfo = dir;
        DWORD64 dirSize = 0;

        // for first element we don't need to move next since it could be directory
        // otherwise we lost size for 1st level directories
        if (dirInfo != nullptr)
        {
            Next();
        }

        while (!IsEnd())
        {
            auto pItem = *_Cursor;

            if (pItem->iType == FileType::FTYPE_FILE)
            {
                dirSize += pItem->iSize;
                Next();
            }
            else if (dirInfo == nullptr || IsSubDir(dirInfo->pPath, pItem->pPath))
            {
                dirSize += CalcDirectorySize(pItem);
                continue;
            }
            else
            {
                break;
            }
        }

        if (dirInfo != nullptr)
        {
            dirInfo->iSize = dirSize;
        }
        
        return dirSize;
    }

    bool IsSubDir(TChar *pParent, TChar *pChild)
    {
        TChar *pFoundPosition = _pStringOperations->StrStr(pChild, pParent);

        return pFoundPosition != nullptr;
    }

    bool IsDirectory(TChar *pFileName)
    {
        return pFileName[_pStringOperations->StrLen(pFileName) - 1] == '\\';
    }

    void GetDirLine(TString &str, TFilePtr &pFileInfo)
    {
        if (g_ViewParam.bDirName)
        {
            if (g_FormatParam.bIndentAll)
            {
                _DirIndent = _pStringOperations->StrNChr(pFileInfo->pPath, '\\') - 1; 
                _DirIndent = (_DirIndent) * g_FormatParam.Width;
            }

            InsertIndent(str, _DirIndent);

            if (g_ViewParam.bApplyToDirs && g_ViewParam.bFullName)
            {
                // print full name including path
                str += _pSourceFolder;
            }

            str += pFileInfo->pPath;

            if (g_ViewParam.bApplyToDirs)
            {
                // make indent to get next column
                InsertIndent(str, _FileNameColWidth + 4 - str.length());

                if (g_ViewParam.bFileName && g_ViewParam.bExt && g_FormatParam.bExtSeparately)
                {
                    InsertIndent(str, _FileExtColWidth + 3);
                }

                // make empty indent, don't print size
                if (g_ViewParam.bFileName && g_ViewParam.bSize && !g_ViewParam.bDirSize)
                {
                    InsertIndent(str, 18);
                }

                // print dir size
                if (g_ViewParam.bDirSize)
                {
                    auto intStr = _pStringOperations->ConvertFileSizeToString(pFileInfo->iSize);
                    size_t len = intStr.length();
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

    }

    void GetFileLine(TString &str, TFilePtr &pFileInfo)
    {
        size_t len = 0;

        if (g_ViewParam.bFileName)
        {
            if (g_FormatParam.bIndentAll || g_FormatParam.bIndentFiles)
            {
                if (pFileInfo->pPath != nullptr)
                {
                    InsertIndent(str, _DirIndent + g_FormatParam.Width);
                }
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
                len = _pStringOperations->StrLen(pFileInfo->pExt);
            }

            if (g_ViewParam.bExt && g_FormatParam.bExtSeparately)
            {
                InsertIndent(str, _FileExtColWidth + 3 - len);
            }

            // print file size
            if (g_ViewParam.bSize)
            {
                auto intStr = _pStringOperations->ConvertFileSizeToString(pFileInfo->iSize);
                len = intStr.length();
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

    void InsertIndent(TString &str, size_t num)
    {
        for (size_t i = 0; i < num; i++)
        {
            str += ' ';
        }
    }

    void InsertChar(TString& str, TChar ch, USHORT num)
    {
        for (USHORT i = 0; i < num; i++)
        {
            str += ch;
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
            str += _pStringOperations->ConvertDateToString(FileTime);
        }

        if (g_ViewParam.bTime)
        {
            str += _pStringOperations->ConvertTimeToString(FileTime);
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
    USHORT CalculateIndent(TChar *pStr)
    {
        if (pStr == nullptr)
        {
            return 0;
        }

        USHORT Indent = 0;

        if (g_FormatParam.bIndentAll)
        {
            Indent = _pStringOperations->StrNChr(pStr, '\\');
            Indent = Indent * g_FormatParam.Width;
        }
        else
            if (g_FormatParam.bIndentFiles && g_ViewParam.bFileName)
            {
                Indent = _pStringOperations->StrNChr(pStr, '\\');
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
    decltype(auto) GetShortFileName(TChar *pFullName)
    {
        TChar *pFileName = _pStringOperations->StrRChr(pFullName, '\\');

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


    /*****************************************************************************
        Routine:     SplitFileName
    ------------------------------------------------------------------------------
        Description:
                    To avoid redundant coping of path and file name, 
                    I just insert zero between them instead last slash.
                    That leads to some overhead during sorting 
                    when I need to compare directories and files

        Arguments:
                    pFullName	- pointer to full file name

        Return Value:
                    Pointer to first short name character in the
                    full file name

    *****************************************************************************/
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

    bool GetFileAttr(TChar *pFileName, LPVOID FileDescriptor);


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
    decltype(auto) CreateDirInfo(TChar *pFileName)
    {
        WIN32_FILE_ATTRIBUTE_DATA FileDescription = {0};
        auto pFileInfo = std::make_unique<TFile>();

        pFileInfo->pPath = pFileName;
        pFileInfo->pName = nullptr;

        if ((g_ViewParam.bDate || g_ViewParam.bTime || g_ViewParam.bAttr) && g_ViewParam.bApplyToDirs)
        {
            GetFileAttr(pFileName, &FileDescription);
        }

        pFileInfo->Attr = FileDescription.dwFileAttributes;
        pFileInfo->DateTime = FileDescription.ftLastWriteTime;
        pFileInfo->iSize = 0;
        pFileInfo->pExt = nullptr;
        pFileInfo->iType = FileType::FTYPE_DIRECTORY;

        auto PathLen = _pStringOperations->StrLen(pFileInfo->pPath);
        auto length = PathLen + CalculateIndent(pFileInfo->pPath);
        if (length > _FileNameColWidth)
        {
            _FileNameColWidth = (USHORT)length;
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
    decltype(auto) CreateFileInfo(TChar *pFileName)
    {
        WIN32_FILE_ATTRIBUTE_DATA FileDescription = {0};

        auto pFileInfo = std::make_unique<TFile>();

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

        auto Length = (USHORT)_pStringOperations->StrLen(pFileName) + CalculateIndent(pFileName);

        pFileInfo->pName = SplitFileName(pFileName, &pFileInfo->pPath);

        pFileInfo->pExt = _pStringOperations->StrRChr(pFileInfo->pName, '.');

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
            ExtLen = (int)_pStringOperations->StrLen(pFileInfo->pExt);
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
bool FileList<FileListItem<char>, char, std::string>::GetFileAttr(char *pFileName, LPVOID FileDescriptor)
{
    return GetFileAttributesExA(
        pFileName,
        GetFileExInfoStandard,
        FileDescriptor
    );
}


template <>
bool FileList<FileListItem<WCHAR>, WCHAR, std::wstring>::GetFileAttr(WCHAR *pFileName, LPVOID FileDescriptor)
{
    return GetFileAttributesExW(
        pFileName,
        GetFileExInfoStandard,
        FileDescriptor
    );
}


