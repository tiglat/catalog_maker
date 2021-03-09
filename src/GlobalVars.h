/*****************************************************************************
**
**  FILE   :	GlobalVars.h
**
**  PURPOSE:	
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#ifndef __GLOBALVARS_H__
#define __GLOBALVARS_H__

#include "windows.h"
#include "wcxhead.h"
#include <string>

#define	SUCCESS				0
#define	OPTION_PAGES		4
#define INDENT_SIZE			3
#define MASK_LIST_LENGTH	50
#define STRING_LENGTH		1000
#define PATH_LENGTH			1000
#define EXT_LENGTH			10

#define COL_NAME	0
#define COL_EXT		1
#define COL_SIZE	2
#define COL_DATE	3
#define COL_TIME	4
#define COL_ATTR	5

#define PLUGIN_NAME "CatalogMaker.wcx"

#define COLUMN_NUMBER	6

typedef struct _TDlgDesc { 
    HWND hTabCtrl;
    HWND hCurDlg;  
    HWND hChildDlg[OPTION_PAGES];
    DLGTEMPLATE *ChildDlgRes[OPTION_PAGES];
} TDlgDesc;

typedef struct _TViewParam{ 
    BOOL	bDirName;
    BOOL    bApplyToDirs;
    BOOL	bFileName;
    BOOL	bFullName;
    BOOL	bExt;
    BOOL	bSize;
    BOOL	bDirSize;
    BOOL	bDate;
    BOOL	bTime;
    BOOL	bAttr;
    char	sFileTypes[MASK_LIST_LENGTH];
} TViewParam;

typedef struct _TSortParam{ 
    BOOL	bName;
    BOOL	bSize;
    BOOL	bDate;
    BOOL	bExt;
    BOOL	bUnsorted;
    BOOL	bDescent;
} TSortParam;

typedef struct _TFormatParam{ 
    BOOL	bIndentFiles;
    BOOL	bIndentAll;
    UINT	Width;
    BOOL	bExtSeparately;
} TFormatParam;

enum FileType
{
    FTYPE_DIRECTORY,
    FTYPE_FILE
};

enum FileEncoding
{
    WRONG,
    ANSI,
    UNICODE
};

#define TYPE_DIRECTORY	0
#define TYPE_FILE		1

typedef struct _TFileList{
    char		*pOriginalName;
    char		*pName;
    char		*pExt;
    UCHAR		iType;
    DWORD64		iSize;
    FILETIME	DateTime;
    DWORD		Attr;
} TFileList;

template <typename TChar> 
struct TFileInfo{
    TChar		Name[PATH_LENGTH];
    DWORD64		iSize;
    DWORD		Attr;
    DWORD		Year;
    DWORD		Month;
    DWORD		Day;
    DWORD		Hour;
    DWORD		Minute;
    DWORD		Second;
};

typedef struct _TRxDesc{
    char		pBuf[STRING_LENGTH + 1];
    char		DirName[STRING_LENGTH + 1];
    USHORT		RootDirLen;
    USHORT		iReadPos;
    USHORT		iWritePos;
    BOOL		bNeedData;
    UCHAR		iThisIsHeader;
    TFileInfo<char>	CurrentFile;
    DWORD		ReturnedLength;
} TRxDesc;


typedef void( TColumnHandler )(
        char *pStr,
        USHORT StrLen,
        TFileInfo<char> *pFileInfo
        );


struct TListInfo{
    TColumnHandler	*HandleFunc;
    USHORT			StartIdx;
    USHORT			Len;
};

// 
extern TViewParam		g_ViewParam;
extern TSortParam		g_SortParam;
extern TFormatParam		g_FormatParam;
extern std::string      g_WildCardPattern;


extern TDlgDesc		g_DlgDesc;		// 
extern TRxDesc		g_RxDesc;
extern TListInfo	g_ListInfo[COLUMN_NUMBER];

extern HINSTANCE		 g_hinst;		// handle to dll instance
extern tProcessDataProc  g_ProcessDataProc;
extern tProcessDataProcW g_ProcessDataProcW;
extern HWND				 g_MainWin;
extern char				 g_pWorkingDir[PATH_LENGTH];
extern char				 g_pCfgFileName[PATH_LENGTH];


#endif

