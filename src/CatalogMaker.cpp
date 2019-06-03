/*****************************************************************************
**
**  FILE   :	CatalogMaker.cpp
**
**  PURPOSE:	Plugin for Windows Commander 5.0
**				Creates file and directory lists
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#include "wcxapi.h"
#include "resource.h" 

#include <string>
#include "stdio.h"
#include "locale.h"
#include "commctrl.h"

#include "GlobalVars.h"
#include "SupportFunc.h"
#include "UIFunc.h"
#include "AnsiStringOperations.h"
#include "WideStringOperations.h"
#include "FileList.h"
#include "CatalogReader.h"

using namespace std;


typedef struct _TReaderDesc {
    BOOL isUnicode;
    CatalogReader<char, std::string>   *pReaderA;
    CatalogReader<WCHAR, std::wstring> *pReaderW;
} TReaderDesc;

TReaderDesc g_CatalogReaderDesc;


/*****************************************************************************
    Routine:     GetPackerCaps
------------------------------------------------------------------------------
    Description: 
        GetPackerCaps tells WinCmd what features your packer plugin supports
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API int STDCALL
    GetPackerCaps ()
{
    int 
        Options =
            PK_CAPS_NEW | 
            PK_CAPS_MODIFY | 
            PK_CAPS_MULTIPLE |
            //PK_CAPS_DELETE |
            PK_CAPS_OPTIONS;

    return Options;
}

/*****************************************************************************
    Routine:     SetupConfigFileNames
------------------------------------------------------------------------------
    Description: 
                This function is used to calculate ini file name 
  
    Arguments:  
                none

    Return Value:
                TRUE - success

*****************************************************************************/
BOOL SetupConfigFileName()
{
    if (strlen(g_pWorkingDir) != 0)
        return TRUE;

    if ( g_hinst == NULL )
    {
        g_hinst = GetModuleHandle(PLUGIN_NAME);
        if ( g_hinst == NULL ) 
            return FALSE;
    }

    DWORD rv = GetModuleFileName( g_hinst, g_pWorkingDir, PATH_LENGTH );
    if ( !rv ) 
        return FALSE;

    if (!RemoveFileName(g_pWorkingDir))
        return FALSE;

    return TRUE;
};

/*****************************************************************************
    Routine:     GetWildCardAsRegexW
------------------------------------------------------------------------------
    Description:
                Converts file mask from config param to wild card as
                regular expression 

    Arguments:
                none

    Return Value:
                regular expression of a wild card

*****************************************************************************/

std::wstring GetWildCardAsRegexW()
{
    WCHAR WildCardPatternWChar[MASK_LIST_LENGTH];
    memset(WildCardPatternWChar, 0, MASK_LIST_LENGTH);
    MultiByteToWideChar(CP_ACP, 0, g_ViewParam.sFileTypes, -1, WildCardPatternWChar, MASK_LIST_LENGTH);

    std::wstring WildCardPattern(WildCardPatternWChar);
    ConvertWildCardToRegexW(WildCardPattern);
    return WildCardPattern;
}

/*****************************************************************************
    Routine:     LoadConfigParams
------------------------------------------------------------------------------
    Description:
        load configuration parameters

    Arguments:

    Return Value:

*****************************************************************************/

void LoadConfigParams()
{
    if (g_hinst == NULL)
    {
        SetupConfigFileName();
        if (!ReadConfigData())
        {
            MessageBox(
                NULL,
                "Configuration data read error. Default settings will be used.",
                "Warning",
                MB_OK
            );
        }
    }
}

/*****************************************************************************
    Routine:     PackFiles
------------------------------------------------------------------------------
    Description:
        PackFiles specifies what should happen when a user creates,
        or adds files to the archive.
        This is ANSI API function. It is used by old versions of TotalCommander.

    Arguments:

    Return Value:

*****************************************************************************/

WCX_API int STDCALL
PackFiles(
    char *PackedFile,
    char *SubPath,
    char *SrcPath,
    char *AddList,
    int Flags
)
{
    WIN32_FIND_DATA FileInfo;

    setlocale(LC_ALL, "[lang]");

    LoadConfigParams();

    // ------- check if the file is exist -----------------------

    auto hFindFile = FindFirstFile(PackedFile, &FileInfo);

    // if file is exist
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindFile);

        auto rv = MessageBox(
            NULL,
            "The file is already exist. Overwrite it?",
            "Warning",
            MB_YESNO || MB_ICONWARNING || MB_SYSTEMMODAL
        );

        if (rv == IDCANCEL)
        {
            return (SUCCESS);
        }
    }

    // ------- open file to write file list-----------------------

    auto hCatalogFile = CreateFile(
        PackedFile,					// file name
        GENERIC_WRITE,				// access mode
        0,							// share mode
        NULL,						// SD
        CREATE_ALWAYS,				// how to create
        FILE_ATTRIBUTE_NORMAL,      // file attributes
        NULL                        // handle to template file
    );

    if (hCatalogFile == INVALID_HANDLE_VALUE)
    {
        return (E_ECREATE);
    }

    // ------- create file list -----------------------

    SetCurrentDirectory(SrcPath);
    int result = SUCCESS;

    try
    {
        string WildCardPattern(g_ViewParam.sFileTypes);
        ConvertWildCardToRegexA(WildCardPattern);
        basic_regex<char> WildCardAsRegex(WildCardPattern, std::regex_constants::icase);

        AnsiStringOperations ops;

        FileList<FileListItem<char>, char, std::string> list(AddList, SrcPath, &ops, WildCardAsRegex);

        DWORD BytesWritten = 0;
        string s = list.GetHeaderLine();
        auto rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.length(), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

        s = list.GetDividingLine();
        rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.length(), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

        for (list.First(); !list.IsEnd(); list.Next())
        {
            // tell Windows Comander what file we are proccessing
            g_ProcessDataProc(list.GetCurrentFileName(), (int)list.GetCurrentFileSize());

            string s = list.GetCurrentFileLine();
            rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.length(), &BytesWritten, NULL);
            if (rv == NULL)
            {
                CloseHandle(hCatalogFile);
                return E_EWRITE;
            }

        }

        s = list.GetFooterLine();
        rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.length(), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

    }
    catch (...)
    {
        result = E_EWRITE;
    }


    CloseHandle(hCatalogFile);
    return (result);
}

/*****************************************************************************
    Routine:     PackFilesW
------------------------------------------------------------------------------
    Description:
        PackFiles specifies what should happen when a user creates,
        or adds files to the archive.
        This is unicode API function. It is called by TotalCommander by default.

    Arguments:

    Return Value:

*****************************************************************************/

WCX_API int STDCALL
PackFilesW(
    WCHAR *PackedFile,
    WCHAR *SubPath,
    WCHAR *SrcPath,
    WCHAR *AddList,
    int Flags
)
{
    setlocale(LC_ALL, "[lang]");

    LoadConfigParams();

    // ------- check if the target file is exist -----------------------

    WIN32_FIND_DATAW FileInfo = {0};
    auto hFindFile = FindFirstFileW(PackedFile, &FileInfo);

    // if file is exist
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindFile);

        auto rv = MessageBox(
            NULL,
            "The file is already exist. Overwrite it?",
            "Warning",
            MB_YESNO ||
            MB_ICONWARNING ||
            MB_SYSTEMMODAL
        );

        if (rv == IDCANCEL)
        {
            return (SUCCESS);
        }
    }

    // ------- open target file to write file list-----------------------

    auto hCatalogFile = CreateFileW(
        PackedFile,					// file name
        GENERIC_WRITE,				// access mode
        0,							// share mode
        NULL,						// SD
        CREATE_ALWAYS,				// how to create
        FILE_ATTRIBUTE_NORMAL,      // file attributes
        NULL                        // handle to template file
    );

    if (hCatalogFile == INVALID_HANDLE_VALUE)
    {
        return (E_ECREATE);
    }

    // ------- create file list -----------------------

    SetCurrentDirectoryW(SrcPath);
    int result = SUCCESS;

    try
    {
        std::basic_regex<WCHAR> WildCardAsRegex(GetWildCardAsRegexW(), std::regex_constants::icase);
        WideStringOperations ops;

        FileList<FileListItem<WCHAR>, WCHAR, std::wstring> list(AddList, SrcPath, &ops, WildCardAsRegex);
        
        DWORD BytesWritten = 0;

        // insert Byte Order Mark (BOM) for UTF-16 LE.
        unsigned char bom[] = {0xFF, 0xFE};

        auto rv = WriteFile(hCatalogFile, bom, sizeof(bom), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

        wstring s = list.GetHeaderLine();
        rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.size() * sizeof(wchar_t), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

        s = list.GetDividingLine();
        rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.size() * sizeof(wchar_t), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

        for (list.First(); !list.IsEnd(); list.Next())
        {
            // tell Windows Comander what file we are proccessing
            g_ProcessDataProcW(list.GetCurrentFileName(), (int)list.GetCurrentFileSize());

            s = list.GetCurrentFileLine();

            rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.size() * sizeof(wchar_t), &BytesWritten, NULL);

            if (rv == NULL)
            {
                CloseHandle(hCatalogFile);
                return E_EWRITE;
            }
        }

        s = list.GetFooterLine();
        rv = WriteFile(hCatalogFile, s.c_str(), (DWORD)s.size() * sizeof(wchar_t), &BytesWritten, NULL);
        if (rv == NULL)
        {
            CloseHandle(hCatalogFile);
            return E_EWRITE;
        }

    }
    catch (...)
    {
        result = E_EWRITE;
    }

    CloseHandle(hCatalogFile);
    return (result);
}


FileEncoding IsUnicodeFile(HANDLE CatalogFile)
{
    DWORD rv;
    DWORD BytesRead = 0;
    USHORT ByteOrderMark = 0xFFFE;
    USHORT ValueFromFile = 0;

    rv = ReadFile(CatalogFile, &ValueFromFile, sizeof(ValueFromFile), &BytesRead, NULL);

    if ( !rv || BytesRead == 0 ) 
    { 
        return FileEncoding::WRONG;
    }


    if (ValueFromFile == ByteOrderMark)
    {
        return FileEncoding::UNICODE;
    }
    else
    {
        SetFilePointer(CatalogFile, 0, NULL, FILE_BEGIN);
        return FileEncoding::ANSI;
    }

}

/*****************************************************************************
    Routine:     OpenArchive
------------------------------------------------------------------------------
    Description:
        ANSI API function. It is used by old versions of TotalCommander.
        OpenArchive should perform all necessary operations
        when an archive is to be opened

    Arguments:

    Return Value:


*****************************************************************************/

WCX_API HANDLE STDCALL
OpenArchive(
    tOpenArchiveData *ArchiveData
)
{
    HANDLE CatalogFile;

    CatalogFile = CreateFile(
        ArchiveData->ArcName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (CatalogFile == INVALID_HANDLE_VALUE)
    {
        ArchiveData->OpenResult = E_BAD_ARCHIVE;
        return 0;
    }

    static AnsiStringOperations ops;
    g_CatalogReaderDesc.isUnicode = false;
    g_CatalogReaderDesc.pReaderW = nullptr;
    g_CatalogReaderDesc.pReaderA = new CatalogReader<char, string>(CatalogFile, &ops);

    return CatalogFile;
}

/*****************************************************************************
    Routine:     OpenArchiveW
------------------------------------------------------------------------------
    Description:
        Unicode API function. It is used by TotalCommander by default.
        OpenArchive should perform all necessary operations
        when an archive is to be opened

    Arguments:

    Return Value:


*****************************************************************************/

WCX_API HANDLE STDCALL
OpenArchiveW(
    tOpenArchiveDataW *ArchiveData
)
{
    HANDLE hFile;

    hFile = CreateFileW(
        ArchiveData->ArcName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hFile = 0;
        ArchiveData->OpenResult = E_BAD_ARCHIVE;
    }

    memset(&g_RxDesc, 0, sizeof(g_RxDesc));

    g_RxDesc.iReadPos = 0;
    g_RxDesc.iWritePos = 0;
    g_RxDesc.bNeedData = TRUE;
    g_RxDesc.DirName[0] = 0;
    g_RxDesc.RootDirLen = 0;
    g_RxDesc.iThisIsHeader = 2;
    g_RxDesc.ReturnedLength = 0;

    for (int i = 0; i < COLUMN_NUMBER; i++)
    {
        g_ListInfo[i].StartIdx = 0;
        g_ListInfo[i].Len = 0;
    }

    return (hFile);
}

int ReadDataBlock(HANDLE hArcData)
{
    int rv;

    rv = ReadFile(
        hArcData,
        &g_RxDesc.pBuf[g_RxDesc.iWritePos],
        STRING_LENGTH - g_RxDesc.iWritePos,
        &g_RxDesc.ReturnedLength,
        NULL
    );

    if (rv &&  g_RxDesc.ReturnedLength == 0)
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
    Routine:     ReadHeader
------------------------------------------------------------------------------
    Description:
                WinCmd calls ReadHeader
                to find out what files are in the archive

    Arguments:

    Return Value:


*****************************************************************************/

WCX_API	int STDCALL
ReadHeader(
    HANDLE hArcData,
    tHeaderData *HeaderData
)
{
    TFileInfo<char> FileInfo;

    if (!g_CatalogReaderDesc.isUnicode && g_CatalogReaderDesc.pReaderA != nullptr)
    {
        int rc = g_CatalogReaderDesc.pReaderA->ReadNext(FileInfo);
        if (rc != SUCCESS)
        {
            return rc;
        }
    }
    else
    {
        return E_UNKNOWN_FORMAT;
    }

    // tell WinCom what file we are processing now
    g_ProcessDataProc(FileInfo.Name, (int)FileInfo.iSize);

    // fill structure for WinComander
    HeaderData->FileAttr = FileInfo.Attr;
    HeaderData->PackSize = (int)FileInfo.iSize;
    HeaderData->UnpSize = (int)FileInfo.iSize;

    g_CatalogReaderDesc.pReaderA->GetFullFileName(FileInfo, HeaderData->FileName);

    // make date and time for Win Com
    if (FileInfo.Year)
    {
        DWORD time =
            FileInfo.Hour << 11 |
            FileInfo.Minute << 5 | FileInfo.Second / 2;

        DWORD date =
            (FileInfo.Year - 80) << 9 |
            (FileInfo.Month + 1) << 5 |
            FileInfo.Day;

        HeaderData->FileTime = date << 16 | time;
    }
    else
    {
        HeaderData->FileTime = 0;
    }

    return SUCCESS;

    //char seps[] = "\n";
    //char *token, *pstr;
    //TFileInfo FileInfo;
    //DWORD	rv;
    //USHORT  len;

    //memset( &FileInfo, 0, sizeof(TFileInfo) );

    //while ( 1 )
    //{
    //    // if we don't have data for analyse
    //    // we should read next block
    //    if ( g_RxDesc.bNeedData )
    //    {
    //        rv = ReadDataBlock( hArcData );

    //        if ( rv != SUCCESS )
    //        {
    //            return( rv );
    //        }

    //        g_RxDesc.bNeedData = FALSE;
    //    }

    //    // get next string from read buffer
    //    token = strtok( &g_RxDesc.pBuf[g_RxDesc.iReadPos], seps );

    //    // if there is no data for analyse 
    //    // then we should read next block
    //    if ( token == NULL )
    //    {
    //        g_RxDesc.bNeedData = TRUE;
    //        g_RxDesc.iReadPos = 0;
    //        g_RxDesc.iWritePos = 0;
    //        continue;
    //    }

    //    // change read position in the buffer
    //    len = (USHORT) strlen( token );
    //    g_RxDesc.iReadPos += len + 1;
    //    
    //    // is it the end of read data block?
    //    // first condition to find the end of file - every token must have 0x0D at the end 
    //    // except last one It is needed to skip "total files and size" string
    //    // second condition to find end of current block of data
    //    if ( token[len-1] != 0x0D || g_RxDesc.iReadPos > g_RxDesc.ReturnedLength+g_RxDesc.iWritePos )
    //    {
    //        // if token is not complete string
    //        // we should store this token and read next block
    //        memcpy( g_RxDesc.pBuf, token, len );
    //        g_RxDesc.iWritePos = len;
    //        g_RxDesc.iReadPos = 0;
    //        g_RxDesc.bNeedData = TRUE;
    //        continue;
    //    }

    //    // skip empty strings
    //    if ( len == 1 && token[0] == '\r' )
    //        continue;

    //    // Analyse string if it is correct
    //    if ( g_RxDesc.iThisIsHeader )
    //    {
    //        if ( HeaderInfoStringParser( token ) == FALSE )
    //            return ( E_BAD_ARCHIVE );
    //        continue;
    //    }
    //    else
    //        FileInfoStringParser( token, &FileInfo );

    //    // tell WinCom what file we are processing now
    //    g_RxDesc.CurrentFile = FileInfo;
    //    g_ProcessDataProc( FileInfo.Name, (int)FileInfo.iSize );

    //    // if current file is directory than store its short name
    //    // to use in future to make full file name
    //    if ( FileInfo.Attr & 0x10 )
    //    {
    //        if ( g_RxDesc.RootDirLen == 0 )
    //        {
    //            GetShortDirName(&FileInfo);
    //        }
    //        else
    //        {
    //            strcpy(g_RxDesc.DirName, &FileInfo.Name[g_RxDesc.RootDirLen]);
    //        }
    //    }

    //    break;
    //}

    //// fill structure for WinComander
    //HeaderData->FileAttr     = FileInfo.Attr;
    //HeaderData->PackSize     = (int)FileInfo.iSize;
    //HeaderData->UnpSize      = (int)FileInfo.iSize;

    //// if file is directory copy its name 
    //if ( FileInfo.Attr & 0x10 )
    //{
    //    strcpy( HeaderData->FileName, g_RxDesc.DirName );
    //}
    //// if file is file :)  build full name for that file
    //else
    //{
    //    strcpy( HeaderData->FileName, g_RxDesc.DirName );
    //    // get short file name
    //    pstr = strrchr( FileInfo.Name, '\\' );
    //    if ( pstr )
    //        strcat( HeaderData->FileName, ++pstr );
    //    else
    //        strcat( HeaderData->FileName, FileInfo.Name );
    //}

    //// make date and time for Win Com
    //if ( FileInfo.Year )
    //{
    //        DWORD time = 
    //            FileInfo.Hour << 11 | 
    //            FileInfo.Minute << 5 | FileInfo.Second / 2;

    //        DWORD date = 
    //            (FileInfo.Year - 80) << 9 | 
    //            (FileInfo.Month + 1) << 5 | 
    //            FileInfo.Day;

    //        HeaderData->FileTime = date << 16 | time;
    //}
    //else
    //    HeaderData->FileTime = 0;

    //return ( SUCCESS );
}

/*****************************************************************************
    Routine:     ReadHeaderEx
------------------------------------------------------------------------------
    Description: 
                WinCmd calls ReadHeader 
                to find out what files are in the archive
                It is used with new version of TotalCmd to support files >2Gb
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API	int STDCALL 
    ReadHeaderEx (
        HANDLE hArcData, 
        tHeaderDataEx *HeaderDataEx
        )
{
    char seps[] = "\n";
    char *token, *pstr;
    TFileInfo<char> FileInfo;
    DWORD	rv;
    USHORT len;

    memset( &FileInfo, 0, sizeof(TFileInfo<char>) );

    while ( 1 )
    {
        // if we don't have data for analyse
        // we should read next block
        if ( g_RxDesc.bNeedData )
        {
            rv = ReadDataBlock( hArcData );

            if ( rv != SUCCESS )
            {
                return( rv );
            }

            g_RxDesc.bNeedData = FALSE;
        }

        // get next string from read buffer
        token = strtok( &g_RxDesc.pBuf[g_RxDesc.iReadPos], seps );

        // if there is no data for analyse 
        // then we should read next block
        if ( token == NULL )
        {
            g_RxDesc.bNeedData = TRUE;
            g_RxDesc.iReadPos = 0;
            g_RxDesc.iWritePos = 0;
            continue;
        }

        // change read position in the buffer
        len = (USHORT) strlen( token );
        g_RxDesc.iReadPos += len + 1;
        
        // is it the end of read data block?
        // first condition to find the end of file - every token must have 0x0D at the end 
        // except last one It is needed to skip "total files and size" string
        // second condition to find end of current block of data
        if ( token[len-1] != 0x0D || g_RxDesc.iReadPos > g_RxDesc.ReturnedLength+g_RxDesc.iWritePos )
        {
            // if token is not a complete text line,
            // we should store this token and read the next block
            memcpy( g_RxDesc.pBuf, token, len );
            g_RxDesc.iWritePos = len;
            g_RxDesc.iReadPos = 0;
            g_RxDesc.bNeedData = TRUE;
            continue;
        }

        // skip empty strings
        if ( len == 1 && token[0] == '\r' )
            continue;

        // Analyze string if it is correct
        if ( g_RxDesc.iThisIsHeader )
        {
            if ( HeaderInfoStringParser( token ) == FALSE )
                return ( E_BAD_ARCHIVE );
            continue;
        }
        else
            FileInfoStringParser( token, &FileInfo );

        // tell WinCom what file we are proccessing now
        g_RxDesc.CurrentFile = FileInfo;
        g_ProcessDataProc( FileInfo.Name, (int)FileInfo.iSize );

        // if current file is directory then store its name
        // to use in future to make full file name
        if ( FileInfo.Attr & 0x10 )
        {
            if ( g_RxDesc.RootDirLen == 0 )
            {
                GetShortDirName(&FileInfo);
            }
            else
            {
                strcpy(g_RxDesc.DirName, &FileInfo.Name[g_RxDesc.RootDirLen]);
            }
        }

        break;
    }

    // fill stucture for WinComander
    HeaderDataEx->FileAttr     = FileInfo.Attr;
    HeaderDataEx->PackSize     = (int)(FileInfo.iSize & 0x00000000FFFFFFFF);
    HeaderDataEx->PackSizeHigh = (int)(FileInfo.iSize >> 32);
    HeaderDataEx->UnpSize      = (int)(FileInfo.iSize & 0x00000000FFFFFFFF);
    HeaderDataEx->UnpSizeHigh  = (int)(FileInfo.iSize >> 32);

    // if file is directory copy its name 
    if ( FileInfo.Attr & 0x10 )
    {
        strcpy( HeaderDataEx->FileName, g_RxDesc.DirName );
    }
    // if file is file :)  build full name for that file
    else
    {
        strcpy( HeaderDataEx->FileName, g_RxDesc.DirName );
        // get short file name
        pstr = strrchr( FileInfo.Name, '\\' );
        if ( pstr )
            strcat( HeaderDataEx->FileName, ++pstr );
        else
            strcat( HeaderDataEx->FileName, FileInfo.Name );
    }

    // make date and time for Win Com
    if ( FileInfo.Year )
    {
            DWORD time = 
                FileInfo.Hour << 11 | 
                FileInfo.Minute << 5 | FileInfo.Second / 2;

            DWORD date = 
                (FileInfo.Year - 80) << 9 | 
                (FileInfo.Month + 1) << 5 | 
                FileInfo.Day;

            HeaderDataEx->FileTime = date << 16 | time;
    }
    else
        HeaderDataEx->FileTime = 0;

    return ( SUCCESS );
}


/*****************************************************************************
    Routine:     ProcessFile
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API int STDCALL 
    ProcessFile (
        HANDLE hArcData, 
        int Operation, 
        char *DestPath, 
        char *DestName
        )
{
    char Dest[MAX_PATH];
    char buffer[512];
    int numread, numwritten;
    FILE *file_source, *file_dest;
    
    if ( Operation == PK_EXTRACT )
    {
        memset( Dest, 0, sizeof( Dest ) );

        if ( DestPath ) strcat( Dest, DestPath );
        if ( DestName ) strcat( Dest, DestName );

        //rc = CopyFile( g_RxDesc.CurrentFile.Name, Dest, FALSE );
        file_source = fopen( g_RxDesc.CurrentFile.Name, "rb" );
        if ( file_source == NULL ) 
            return( E_NO_FILES );

        file_dest = fopen( Dest, "wb" );
        if ( file_dest == NULL )
        {
            fclose( file_source );
            return( E_ECREATE );
        }

        while( !feof( file_source ) )
        {
            numread = (int)fread( buffer, sizeof( char ), 512, file_source );
            if( ferror( file_source ) )
            {
                fclose( file_source );
                fclose( file_dest );
                return( E_EREAD );
            }
            
            numwritten = (int)fwrite( buffer, sizeof( char ), numread, file_dest );
            if( ferror( file_dest ) ) 
            {
                fclose( file_source );
                fclose( file_dest );
                return( E_EWRITE );
            }

            g_ProcessDataProc( g_RxDesc.CurrentFile.Name, numwritten );
        }
        
        fclose( file_source );
        fclose( file_dest );

        return( SUCCESS );
    }
    else if ( Operation == PK_TEST )
    {
        return ( SUCCESS );
    }
    else
        return ( SUCCESS );
}

/*****************************************************************************
    Routine:     CloseArchive
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API int STDCALL 
    CloseArchive (
        HANDLE hArcData
        )
{
    HRESULT rv;
    UCHAR	Ret = E_ECLOSE;

    rv = CloseHandle( hArcData );

    if (rv == S_OK)
    {
        Ret = SUCCESS;
    }

    if (g_CatalogReaderDesc.pReaderW != nullptr)
    {
        delete g_CatalogReaderDesc.pReaderW;
    }
    
    if (g_CatalogReaderDesc.pReaderA != nullptr)
    {
        delete g_CatalogReaderDesc.pReaderA;
    }

    return Ret;
}

/*****************************************************************************
    Routine:     SetChangeVolProc
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API void STDCALL
    SetChangeVolProc (
        HANDLE hArcData, 
        tChangeVolProc pChangeVolProc1
        )
{
    return;
}

/*****************************************************************************
    Routine:     SetProcessDataProc
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API void STDCALL
    SetProcessDataProc (
        HANDLE hArcData, 
        tProcessDataProc pProcessDataProc
        )
{
    g_ProcessDataProc = pProcessDataProc;
    return;
}

/*****************************************************************************
    Routine:     SetProcessDataProcW
------------------------------------------------------------------------------
    Description:

    Arguments:

    Return Value:


*****************************************************************************/

WCX_API void STDCALL
SetProcessDataProcW(
    HANDLE hArcData,
    tProcessDataProcW pProcessDataProc
)
{
    g_ProcessDataProcW = pProcessDataProc;
    return;
}

/*****************************************************************************
    Routine:     ConfigurePacker
------------------------------------------------------------------------------
    Description: 
            ConfigurePacker gets called when the user clicks the Configure 
            button from within "Pack files..." dialog box in WinCmd
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API void STDCALL
    ConfigurePacker (
        HWND Parent, 
        HINSTANCE DllInstance
        )
{
    INT_PTR	nResult;
    g_hinst = DllInstance;
    
    // SetupConfigFileName() is called to support old versions of TotalCmd < 5.51
    // when it was needed to define config path name ourself.
    // Now config path is provided by TotalCmd via PackSetDefaultParams API function call (see below).
    if ( !SetupConfigFileName() )
        MessageBox(	NULL, "Impossible to write options on your hard disk. Wrong path to configuration file.", "Warning", MB_OK );
    else
        nResult = DialogBox(
                DllInstance, 
                MAKEINTRESOURCE( IDD_OPTIONSDLG ), 
                Parent, 
                (DLGPROC) OptionsDialogProc
                );
}


/*****************************************************************************
    Routine:     PackSetDefaultParams
------------------------------------------------------------------------------
    Description: 
            PackSetDefaultParams is called immediately after loading the DLL,
            before any other function. This function is new in version 2.1.
            It requires Total Commander >=5.51, but is ignored by older versions.
  
    Arguments:  

    Return Value:


*****************************************************************************/

WCX_API void STDCALL
    PackSetDefaultParams (
    PackDefaultParamStruct* dps
    )
{
    strncpy(g_pWorkingDir, dps->DefaultIniName, MAX_PATH-1);

    if (!RemoveFileName(g_pWorkingDir))
    {
        // we need to reset working directory path to produce a new path in ConfigurePacker()
        strcpy(g_pWorkingDir, "");
    }
}
