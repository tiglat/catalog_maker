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

#include "GlobalVars.h"
#include "SupportFunc.h"
#include "UIFunc.h"
#include "wcxapi.h"
#include "resource.h" 

#include "stdio.h"
#include "locale.h"
#include "commctrl.h"

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
	Routine:     CompareFileDesc 
------------------------------------------------------------------------------
	Description: 
				This function is used to sort file list
				It compares two file descriptors
  
	Arguments:  
				p1	- pointer to first descriptor
				p2	- pointer to second descriptor

	Return Value:
				if p1 < p2 then returns negative number
				if p1 > p2 then returns positive number
				if p1 = p2 then returns zero

*****************************************************************************/

int CompareFileDesc( const void *p1, const void *p2 )
{
	TFileList *pItem1 = (TFileList*) p1;
	TFileList *pItem2 = (TFileList*) p2;
	long	rv;
	char *pName1, *pName2;
	char *pPath1, *pPath2;

	// get file name and path from first Item
	pName1  = strrchr( pItem1->pOriginalName, '\\' );
	if ( pName1 )
	{
		*pName1 = 0;

		if ( pItem1->iType == TYPE_FILE )
			pName1++;
		else
			pName1 = NULL;

		pPath1 = pItem1->pOriginalName;
	}
	else
	{
		pName1 = pItem1->pOriginalName;
		pPath1 = NULL;
	}

	// get file name and path from second Item
	pName2  = strrchr( pItem2->pOriginalName, '\\' );
	if ( pName2 )
	{
		*pName2 = 0;

		if ( pItem2->iType == TYPE_FILE )
			pName2++;
		else
			pName2 = NULL;

		pPath2 = pItem2->pOriginalName;
	}
	else
	{
		pName2 = pItem2->pOriginalName;
		pPath2 = NULL;
	}

	// compare paths and then compare another attributes
	// depending on settings
	if ( pPath1 && pPath2 )
	{
		rv = _stricmp( pPath1, pPath2 );
		pPath1[ strlen(pPath1) ] = '\\';
		pPath2[ strlen(pPath2) ] = '\\';
	}
	else
	{
		if ( pPath1 == 0 && pPath2 == 0 )
		{ 
			rv = 0; 
		}
		else
		{
			if ( pPath1 )
				pPath1[ strlen(pPath1) ] = '\\';

			if ( pPath2 )
				pPath2[ strlen(pPath2) ] = '\\';

			if ( pPath1 == 0 )
				rv = -1;
			else
				rv = 1;
		}
	}

	// if paths are equal then we need to compare another attributes
	if ( rv == 0 )
	{
		// if one of the items is directory than it is always less
		if ( pName1 == 0 )
		{
			rv = -1;
			return ( rv );
		}
		else
			if ( pName2 == 0 )
			{
				rv = 1;
				return ( rv );
			}

		// if user selected sort by extension
		if ( g_SortParam.bExt )
		{
			if ( pItem1->pExt == 0 )
				rv = -1;

			if ( pItem2->pExt == 0 )
				rv = 1;

			if ( pItem1->pExt && pItem2->pExt )
				rv = _stricmp( pItem1->pExt, pItem2->pExt );
		}

		// if user selected sort by size
		if ( g_SortParam.bSize )
		{
			if ( pItem1->iSize < pItem2->iSize )
			{
				rv = -1;
			}
			else
			{
				if ( pItem1->iSize > pItem2->iSize )
					rv = 1;
				else
					rv = 0;
			}
		}

		// if user selected sort by date
		if ( g_SortParam.bDate )
		{
			rv = CompareFileTime( 
					&pItem1->DateTime,
					&pItem2->DateTime
					);
		}

		// if user selected sort by name
		if ( g_SortParam.bName || rv == 0 )
		{
			if ( pName1 == 0 )
				rv = -1;

			if ( pName2 == 0 )
				rv = 1;

			if ( pName1 && pName2 )
				//rv = _wcsicmp( (wchar_t*)pName1, (wchar_t*)pName2 );
				rv = _stricmp( pName1, pName2 );
		}

		if ( g_SortParam.bDescent )
		{
			rv *= -1;
		}	

	} // if rv
		
	return ( rv );
}

/*****************************************************************************
	Routine:     CreateFileList 
------------------------------------------------------------------------------
	Description: 
				Reads Windows Commander file list AddList and
				creates array of file descriptors
  
	Arguments:  
		AddList			-	pointer to string containing file list
							devided zero 
		SourceFolder	-	pointer to string with Source Folder

	Return Value:
		pFileList		-	pointer to array of descriptors
		CountItems		-	number of descriptors in the pFileList
		MaxLen			-	the length of longest file name

		The function returns also the code of error

*****************************************************************************/

DWORD CreateFileList ( 
		char		*AddList, 
		char		*SourceFolder,
		TFileList	**pFileList, 
		DWORD		*CountItems,
		USHORT		*MaxLen,
		USHORT		*MaxExtLen
		)
{
	char		*token = AddList, *pShortFileName;
	ULONG		i = 0, ReturnCode;
	TFileList	*pFileItem;
	USHORT		length = 0, Indent = 0, ExtLen;
	WIN32_FILE_ATTRIBUTE_DATA FileInfo;

	// count number of items in the AddList
	while ( AddList[i] )
	{
		while ( AddList[i] ) i++;
		(*CountItems)++;
		i++;
	}

	// allocate memory for file name descriptor list
	pFileItem = *pFileList = 
		(TFileList*) malloc( sizeof(TFileList) * (*CountItems) );

	if ( pFileItem == NULL )
	{
		return ( E_NO_MEMORY );
	}

	SetCurrentDirectory( SourceFolder );

	// we must return number of descriptors in the created list
	*CountItems = 0;

	// analyse AddList
	while ( strlen( token ) != 0 )
	{
		// if Item is derictory name
		if ( token[strlen(token)-1] == '\\' )
		{
			if ( g_ViewParam.bDirName )
			{
				pFileItem->pOriginalName = token;
				pFileItem->pName = token;

				ReturnCode = GetFileAttributesEx(
					token,
					GetFileExInfoStandard,
					&FileInfo
				);

				pFileItem->Attr		= FileInfo.dwFileAttributes;
				pFileItem->DateTime = FileInfo.ftLastWriteTime;
				pFileItem->iSize = 0;
//				pFileItem->Attr  = 0;
//				pFileItem->pExt  = 0;
				pFileItem->iType = TYPE_DIRECTORY;

				Indent = CalculateIndent( pFileItem->pOriginalName );
				length = (USHORT) strlen ( pFileItem->pName ) + Indent;

				if ( length > *MaxLen )
					*MaxLen = length;

				pFileItem++;
				(*CountItems)++;
			}
		}
		// if Item is file name 
		else
		{
			if ( g_ViewParam.bFileName )
			{
				pShortFileName = GetShortFileName( token );

				// check if the file name is matched to mask list
				if ( CompareFileNameWithMaskList( 
					pShortFileName,
					g_ViewParam.sFileTypes ) )
				{
					ReturnCode = GetFileAttributesEx(
						token,
						GetFileExInfoStandard,
						&FileInfo
					);

					pFileItem->Attr		= FileInfo.dwFileAttributes;
					pFileItem->DateTime = FileInfo.ftLastWriteTime;
					pFileItem->iSize	= FileInfo.nFileSizeHigh;
					pFileItem->iSize	= pFileItem->iSize << 32 | FileInfo.nFileSizeLow;
					pFileItem->iType	= TYPE_FILE;

					if ( ReturnCode == 0 )
						pFileItem->iSize = 0;
					
					pFileItem->pOriginalName = token;
		
					if ( g_ViewParam.bFullName )
						pFileItem->pName = token;
					else
						pFileItem->pName = pShortFileName;

					pFileItem->pExt = strrchr( pShortFileName, '.' );

					if ( pFileItem->pExt == pShortFileName )
						pFileItem->pExt = 0;

					ExtLen = 0;
					if ( pFileItem->pExt )
					{
						*pFileItem->pExt = 0;
						token = ++pFileItem->pExt;
						ExtLen = (USHORT) strlen ( pFileItem->pExt );
						if ( ExtLen > *MaxExtLen )
						{
							*MaxExtLen = ExtLen;
						}
					}

					Indent = CalculateIndent( pFileItem->pOriginalName );

					length = (USHORT) strlen ( pFileItem->pName ) + Indent;

					if ( g_ViewParam.bExt && !g_FormatParam.bExtSeparately )
						length  += ExtLen + 1;

					if ( length > *MaxLen )
						*MaxLen = length;

					pFileItem++;
					(*CountItems)++;
				}
			}
		}
		
		token = &token[strlen( token )+1];
	}

	// increase MaxLen if we will include full file name
	if ( g_ViewParam.bFullName )
		*MaxLen += (USHORT) strlen( SourceFolder );

	return( 0 );
}

/*****************************************************************************
	Routine:     PackFiles
------------------------------------------------------------------------------
	Description: 
		PackFiles specifies what should happen when a user creates, 
		or adds files to the archive.
  
	Arguments:  

	Return Value:

*****************************************************************************/

WCX_API int STDCALL 
	PackFiles (
		char *PackedFile, 
		char *SubPath, 
		char *SrcPath, 
		char *AddList, 
		int Flags
		)
{
	TFileList	*pFileList = 0, *pFileItem;
	HANDLE		hCatalogFile, hFindFile;
	DWORD		ErrorCode, CountItems = 0, j;
    DWORD		idx, len, idxDirIndent = 0, idxFileIndent = 0;
    DWORD		ReturnedLength, rv;
	DWORD64		TotalSize = 0;
	DWORD		TotalFiles = 0;
	char		pBuf[STRING_LENGTH];
	USHORT		MaxLen = 9, MaxExtLen = 3;
	SYSTEMTIME	FileTime;
	WIN32_FIND_DATA FileInfo;

	setlocale( LC_ALL, "[lang]" );

	if ( g_hinst == NULL )
	{
		SetupConfigFileName();
		if ( !ReadConfigData() )
		{
			MessageBox(
				NULL,
				"Configuration data read error. Default settings will be used.",
				"Warning",
				MB_OK
			);
		}
	}

	// ------- check if the file is exist -----------------------

	hFindFile = FindFirstFile(
		PackedFile,
		&FileInfo
	);

	// if file is exist
	if ( hFindFile != INVALID_HANDLE_VALUE  )
	{
		FindClose( hFindFile );

		rv = MessageBox(
			NULL,
			"The file is already exist. Overwrite it?",
			"Warning",
			MB_YESNO ||
			MB_ICONWARNING ||
			MB_SYSTEMMODAL
		);

		if  ( rv == IDCANCEL )
			return ( SUCCESS);
	}

	// ------- open file to write file list-----------------------

	hCatalogFile = CreateFile(
		PackedFile,					// file name
		GENERIC_WRITE,				// access mode
		0,							// share mode
		NULL,						// SD
		CREATE_ALWAYS,				// how to create
		FILE_ATTRIBUTE_NORMAL,      // file attributes
		NULL                        // handle to template file
	);

	if ( hCatalogFile == INVALID_HANDLE_VALUE )
	{
		return ( E_ECREATE );
	}

	// ---------------- create file list from AddList --------------
	ErrorCode = CreateFileList( 
					AddList,
					SrcPath,
					&pFileList, 
					&CountItems, 
					&MaxLen,
					&MaxExtLen
					);

	if ( ErrorCode != 0 )
    {
		CloseHandle( hCatalogFile );
		return ( ErrorCode );
    }

	//----------- sort file list ---------------------------------

	if ( !g_SortParam.bUnsorted )
		qsort( 
			pFileList, 
			CountItems, 
			sizeof( TFileList ), 
			CompareFileDesc 
			);

	//----------- write file list to target file -----------------

	// create header for list
	memset( pBuf, 32, sizeof(pBuf) );
	pBuf[sizeof(pBuf)-1] = 0;

	sprintf( pBuf, "File name" );
	pBuf[strlen( pBuf )] = 32;

	idx = MaxLen + 4;

	if ( g_ViewParam.bFileName )
	{
		if ( g_ViewParam.bExt && g_FormatParam.bExtSeparately )
		{
			idx += sprintf( pBuf + idx, "Ext" );
			pBuf[idx] = 32;
			idx += MaxExtLen;
		}

		if ( g_ViewParam.bSize )
		{
			idx += sprintf( pBuf + idx, "Size   " );
			pBuf[idx] = 32;
			idx += 11;
		}

		if ( g_ViewParam.bDate )
			idx += sprintf( pBuf + idx, "Date        " );

		if ( g_ViewParam.bTime )
			idx += sprintf( pBuf + idx, "Time      " );

		if ( g_ViewParam.bAttr )
			idx += sprintf( pBuf + idx, "Attr" );
	}
	else if ( g_ViewParam.bDirName && g_ViewParam.bApplyToDirs )
	{
		if ( g_ViewParam.bDate )
			idx += sprintf( pBuf + idx, "Date        " );

		if ( g_ViewParam.bTime )
			idx += sprintf( pBuf + idx, "Time      " );

		if ( g_ViewParam.bAttr )
			idx += sprintf( pBuf + idx, "Attr" );
	}

	idx += sprintf( pBuf + idx, "\r\n" );

	rv = WriteFile( 
		hCatalogFile,
		pBuf,
		idx,
		&ReturnedLength,
		NULL
		);

	if ( rv == NULL )
	{
		free( pFileList );
		CloseHandle( hCatalogFile );
		return ( E_EWRITE );
	}

	memset( pBuf, 32, sizeof(pBuf) );
	pBuf[sizeof(pBuf)-1] = 0;

	memset( pBuf, '-', MaxLen );
	idx = MaxLen + 4;

	if ( g_ViewParam.bFileName )
	{
		if ( g_ViewParam.bExt && g_FormatParam.bExtSeparately )
		{
			memset( pBuf + idx, '-', MaxExtLen );
			idx += MaxExtLen + 3;
		}

		if ( g_ViewParam.bSize )
			idx += sprintf( pBuf + idx, "---------------   " );

		if ( g_ViewParam.bDate )
			idx += sprintf( pBuf + idx, "----------  " );

		if ( g_ViewParam.bTime )
			idx += sprintf( pBuf + idx, "--------  " );

		if ( g_ViewParam.bAttr )
			idx += sprintf( pBuf + idx, "----" );
	}
	else if ( g_ViewParam.bDirName && g_ViewParam.bApplyToDirs )
	{
		if ( g_ViewParam.bDate )
			idx += sprintf( pBuf + idx, "----------  " );

		if ( g_ViewParam.bTime )
			idx += sprintf( pBuf + idx, "--------  " );

		if ( g_ViewParam.bAttr )
			idx += sprintf( pBuf + idx, "----" );
	}

	idx += sprintf( pBuf + idx, "\r\n" );

	rv = WriteFile( 
		hCatalogFile,
		pBuf,
		idx,
		&ReturnedLength,
		NULL
		);

	if ( rv == NULL )
	{
		free( pFileList );
		CloseHandle( hCatalogFile );
		return ( E_EWRITE );
	}
	
	pFileItem = pFileList;

	for ( j = 0; j < CountItems; j++ )
	{
		idx = 0;
		memset( pBuf, 32, sizeof(pBuf) );
		pBuf[sizeof(pBuf)-1] = 0;

		// tell Windows Comander what file we are proccessing
		g_ProcessDataProc( pFileItem->pName, (int) pFileItem->iSize );

		if ( pFileItem->iType == TYPE_DIRECTORY )
		{
			if ( g_ViewParam.bDirName )
			{
				if ( g_FormatParam.bIndentFiles )
					idxFileIndent = g_FormatParam.Width;

				if ( g_FormatParam.bIndentAll )
				{
					idxDirIndent = strnchr( pFileItem->pName, '\\' );
					idx = idxDirIndent = 
						(idxDirIndent-1) * g_FormatParam.Width;
				}

                if ( g_ViewParam.bApplyToDirs && g_ViewParam.bFullName )
                {
    				// print full name including path
   					idx += sprintf( pBuf + idx, "%s", SrcPath );
                }

				len = (DWORD) strlen( pFileItem->pName );
				memcpy( pBuf + idx, pFileItem->pName, len );
				idx += len;

                if ( g_ViewParam.bApplyToDirs )
                {
                    // make indent to get next column
    				idx = MaxLen + 4;
                    
					if ( g_ViewParam.bFileName && g_ViewParam.bExt && g_FormatParam.bExtSeparately )
    				{
    					idx += MaxExtLen + 3;
    				}

    				// make empty indent, don't print size
    				if ( g_ViewParam.bFileName && g_ViewParam.bSize )
    				{
    					idx += 18;
    				}
                    
    				// print file date and time
    				if ( g_ViewParam.bDate || g_ViewParam.bTime )
    				{
    					rv = FileTimeToLocalFileTime( 
    							&pFileItem->DateTime,
    							&pFileItem->DateTime 
    							);

    					rv = FileTimeToSystemTime( 
    							&pFileItem->DateTime, 
    							&FileTime 
    							);
    				}

    				if ( g_ViewParam.bDate )
    				{
    					idx += sprintf( pBuf + idx, "%02d.", FileTime.wDay );
    					idx += sprintf( pBuf + idx, "%02d.", FileTime.wMonth );
    					idx += sprintf( pBuf + idx, "%4d  ", FileTime.wYear );
    				}

    				if ( g_ViewParam.bTime )
    				{
    					idx += sprintf( pBuf + idx, "%02d:", FileTime.wHour );
    					idx += sprintf( pBuf + idx, "%02d.", FileTime.wMinute );
    					idx += sprintf( pBuf + idx, "%02d  ", FileTime.wSecond );
    				}

    				// print file attributes
    				if ( g_ViewParam.bAttr )
    				{
    					if ( pFileItem->Attr & FILE_ATTRIBUTE_READONLY )
    						idx += sprintf( pBuf + idx, "r" );
    					else
    						idx += sprintf( pBuf + idx, "-" );

    					if ( pFileItem->Attr & FILE_ATTRIBUTE_ARCHIVE )
    						idx += sprintf( pBuf + idx, "a" );
    					else
    						idx += sprintf( pBuf + idx, "-" );
    					
    					if ( pFileItem->Attr & FILE_ATTRIBUTE_HIDDEN )
    						idx += sprintf( pBuf + idx, "h" );
    					else
    						idx += sprintf( pBuf + idx, "-" );

    					if ( pFileItem->Attr & FILE_ATTRIBUTE_SYSTEM )
    						idx += sprintf( pBuf + idx, "s" );
    					else
    						idx += sprintf( pBuf + idx, "-" );
    				}
                }            
            }
		}
		else
		{
			if ( g_ViewParam.bFileName )
			{
				TotalFiles++;

				if ( g_FormatParam.bIndentAll || g_FormatParam.bIndentFiles )
					idx = idxDirIndent + idxFileIndent; 

				// print file path
				if ( g_ViewParam.bFullName )
					idx += sprintf( pBuf + idx, "%s", SrcPath );

				// print file name
				len = (DWORD) strlen( pFileItem->pName );
				memcpy( pBuf + idx, pFileItem->pName, len );
				idx += len;

				// print file extension with file name
				if ( g_ViewParam.bExt && pFileItem->pExt && 
					!g_FormatParam.bExtSeparately )
				{
					pBuf[idx++] = '.';
					memcpy( 
						pBuf + idx, 
						pFileItem->pExt, 
						strlen(pFileItem->pExt) 
						);
				}

				idx = MaxLen + 4;
				
				// print file extension separately
				if ( g_ViewParam.bExt && pFileItem->pExt && 
					g_FormatParam.bExtSeparately )
				{
					memcpy( 
						pBuf + idx, 
						pFileItem->pExt, 
						strlen(pFileItem->pExt) 
						);
				}

				if ( g_ViewParam.bExt && g_FormatParam.bExtSeparately )
				{
					idx += MaxExtLen + 3;
				}

				// print file size
				if ( g_ViewParam.bSize )
				{
					TotalSize += pFileItem->iSize;
					FormatIntNumber( 
						pBuf + idx, 
						pFileItem->iSize 
						);
					idx += 18;
				}

				// print file date and time
				if ( g_ViewParam.bDate || g_ViewParam.bTime )
				{
					rv = FileTimeToLocalFileTime( 
							&pFileItem->DateTime,
							&pFileItem->DateTime 
							);

					rv = FileTimeToSystemTime( 
							&pFileItem->DateTime, 
							&FileTime 
							);
				}

				if ( g_ViewParam.bDate )
				{
					idx += sprintf( pBuf + idx, "%02d.", FileTime.wDay );
					idx += sprintf( pBuf + idx, "%02d.", FileTime.wMonth );
					idx += sprintf( pBuf + idx, "%4d  ", FileTime.wYear );
				}

				if ( g_ViewParam.bTime )
				{
					idx += sprintf( pBuf + idx, "%02d:", FileTime.wHour );
					idx += sprintf( pBuf + idx, "%02d.", FileTime.wMinute );
					idx += sprintf( pBuf + idx, "%02d  ", FileTime.wSecond );
				}

				// print file attributes
				if ( g_ViewParam.bAttr )
				{
					if ( pFileItem->Attr & FILE_ATTRIBUTE_READONLY )
						idx += sprintf( pBuf + idx, "r" );
					else
						idx += sprintf( pBuf + idx, "-" );

					if ( pFileItem->Attr & FILE_ATTRIBUTE_ARCHIVE )
						idx += sprintf( pBuf + idx, "a" );
					else
						idx += sprintf( pBuf + idx, "-" );
					
					if ( pFileItem->Attr & FILE_ATTRIBUTE_HIDDEN )
						idx += sprintf( pBuf + idx, "h" );
					else
						idx += sprintf( pBuf + idx, "-" );

					if ( pFileItem->Attr & FILE_ATTRIBUTE_SYSTEM )
						idx += sprintf( pBuf + idx, "s" );
					else
						idx += sprintf( pBuf + idx, "-" );
				}
			}
		}

		idx += sprintf( pBuf + idx, "\r\n" );
		pFileItem++;

		rv = WriteFile( 
			hCatalogFile,
			pBuf,
			idx,
			&ReturnedLength,
			NULL
			);

		if ( rv == NULL )
		{
			free( pFileList );
			CloseHandle( hCatalogFile );
			return ( E_EWRITE );
		}
	}

	// print total size and number of the files
	if ( g_ViewParam.bFileName )
	{
		memset( pBuf, 32, sizeof(pBuf) );
		pBuf[sizeof(pBuf)-1] = 0;

		idx = 0;
		idx = sprintf( pBuf, "\r\ntotal files %d", TotalFiles );

		if ( g_ViewParam.bSize )
		{
			idx += sprintf( pBuf + idx, "    total size " );
			pBuf[strlen(pBuf)] = 32;
			idx += FormatIntNumber( pBuf + idx, TotalSize );
		}

		// don't include symbol '\r' in order to read 
		// the list file easier
		idx += sprintf( pBuf + idx, "\n" );

		rv = WriteFile( 
			hCatalogFile,
			pBuf,
			idx,
			&ReturnedLength,
			NULL
			);

		if ( rv == NULL )
		{
			free( pFileList );
			CloseHandle( hCatalogFile );
			return ( E_EWRITE );
		}
	}

	// ----------  finish work ---------------------

	free( pFileList );
	CloseHandle( hCatalogFile );
	return ( SUCCESS );
}

/*****************************************************************************
	Routine:     OpenArchive
------------------------------------------------------------------------------
	Description: 
				OpenArchive should perform all 
				necessary operations when an archive is to be opened

	Arguments:  

	Return Value:


*****************************************************************************/

WCX_API HANDLE STDCALL 
	OpenArchive (
		tOpenArchiveData *ArchiveData
		)
{
	HANDLE hFile;

	hFile = CreateFile(
		ArchiveData->ArcName,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

    if ( hFile == INVALID_HANDLE_VALUE )
    {
		hFile = 0;
		ArchiveData->OpenResult = E_BAD_ARCHIVE;
    }

	memset( &g_RxDesc, 0, sizeof(g_RxDesc) );

	g_RxDesc.iReadPos	= 0;
	g_RxDesc.iWritePos	= 0;
	g_RxDesc.bNeedData	= TRUE;
	g_RxDesc.DirName[0] = 0;
	g_RxDesc.RootDirLen = 0;
	g_RxDesc.iThisIsHeader	= 2;
	g_RxDesc.ReturnedLength = 0;

	for ( int i = 0; i < COLUMN_NUMBER; i++ )
	{
		g_ListInfo[i].StartIdx	= 0;
		g_ListInfo[i].Len		= 0;
	}

	return ( hFile );
}

/*****************************************************************************
	Routine:     ReadHeader
------------------------------------------------------------------------------
	Description: 
  
	Arguments:  

	Return Value:


*****************************************************************************/

DWORD ReadDataBlock( HANDLE hArcData )
{
	DWORD rv;

	rv = ReadFile( 
		hArcData,
		&g_RxDesc.pBuf[g_RxDesc.iWritePos],
		STRING_LENGTH-g_RxDesc.iWritePos,
		&g_RxDesc.ReturnedLength,
		NULL
		);

	if ( rv &&  g_RxDesc.ReturnedLength == 0 ) 
	{ 
		return ( E_END_ARCHIVE );
	}

	if ( rv == NULL  )
	{
		return ( E_EREAD );
	}

	return ( SUCCESS );
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
	TFileInfo FileInfo;
	DWORD	rv;
	USHORT len;

	memset( &FileInfo, 0, sizeof(TFileInfo) );

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
	Routine:     ReadHeader
------------------------------------------------------------------------------
	Description: 
				WinCmd calls ReadHeader 
				to find out what files are in the archive

	Arguments:  

	Return Value:


*****************************************************************************/

WCX_API	int STDCALL 
	ReadHeader (
		HANDLE hArcData, 
		tHeaderData *HeaderData
		)
{
	char seps[] = "\n";
	char *token, *pstr;
	TFileInfo FileInfo;
	DWORD	rv;
	USHORT  len;

	memset( &FileInfo, 0, sizeof(TFileInfo) );

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
			// if token is not complete string
			// we should store this token and read next block
			memcpy( g_RxDesc.pBuf, token, len );
			g_RxDesc.iWritePos = len;
			g_RxDesc.iReadPos = 0;
			g_RxDesc.bNeedData = TRUE;
			continue;
		}

		// skip empty strings
		if ( len == 1 && token[0] == '\r' )
			continue;

		// Analyse string if it is correct
		if ( g_RxDesc.iThisIsHeader )
		{
			if ( HeaderInfoStringParser( token ) == FALSE )
				return ( E_BAD_ARCHIVE );
			continue;
		}
		else
			FileInfoStringParser( token, &FileInfo );

		// tell WinCom what file we are processing now
		g_RxDesc.CurrentFile = FileInfo;
		g_ProcessDataProc( FileInfo.Name, (int)FileInfo.iSize );

		// if current file is directory than store its short name
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

	// fill structure for WinComander
	HeaderData->FileAttr     = FileInfo.Attr;
	HeaderData->PackSize     = (int)FileInfo.iSize;
	HeaderData->UnpSize      = (int)FileInfo.iSize;

	// if file is directory copy its name 
	if ( FileInfo.Attr & 0x10 )
	{
		strcpy( HeaderData->FileName, g_RxDesc.DirName );
	}
	// if file is file :)  build full name for that file
	else
	{
		strcpy( HeaderData->FileName, g_RxDesc.DirName );
		// get short file name
		pstr = strrchr( FileInfo.Name, '\\' );
		if ( pstr )
			strcat( HeaderData->FileName, ++pstr );
		else
			strcat( HeaderData->FileName, FileInfo.Name );
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

			HeaderData->FileTime = date << 16 | time;
	}
	else
		HeaderData->FileTime = 0;

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

	if ( rv == S_OK )
		Ret = SUCCESS;

	return ( Ret );
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
