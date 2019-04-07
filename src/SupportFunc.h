/*****************************************************************************
**
**  FILE   :	SupportFunc.h
**
**  PURPOSE:	
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#ifndef __SUPPORTFUNC_H__
#define __SUPPORTFUNC_H__

#include "windows.h"
#include "GlobalVars.h"

USHORT CalculateIndent( char *pStr );
BOOL IsStrValid( char *pStr, char *pMask );
char *GetShortFileName( char *pFullName );
BOOL IsFileNameValid( char *pFileName, char *pMask );
BOOL CompareFileNameWithMaskList( char *pFileName, char *pMaskList );
DWORD FormatIntNumber( char *pResultStr, DWORD64 Number );
UINT strnchr( char *pStr, char ch );
void FileInfoStringParser( char *Str, TFileInfo *FileInfo );
BOOL HeaderInfoStringParser( char *pStr );
void DelSpacesAroundStr ( char *pResultStr, char *pStr, USHORT Len );
BOOL ReadConfigData();
void GetShortDirName(TFileInfo *pFileInfo);
BOOL RemoveFileName(char *pFullPath);
void ConvertWildCardToRegex(std::string &pattern);




#endif


