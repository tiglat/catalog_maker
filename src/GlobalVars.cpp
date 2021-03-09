/*****************************************************************************
**
**  FILE   :	GlobalVars.cpp
**
**  PURPOSE:	
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#include "GlobalVars.h"

// 
TViewParam		g_ViewParam = 
{ 
    TRUE,   // bDirName
    TRUE,   // bApplyToDirs
    TRUE,   // bFileName
    FALSE,  // bFullName
    TRUE,   // bExt
    TRUE,   // bSize
    FALSE,  // bDirSize;
    FALSE,  // bDate;
    FALSE,  // bTime;
    FALSE,  // bAttr;
    "*.*"   // sFileTypes
};

TSortParam		g_SortParam = 
		{ TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };
TFormatParam	g_FormatParam = 
		{ TRUE, FALSE, 3, FALSE };

TDlgDesc		g_DlgDesc;		// 
TRxDesc			g_RxDesc;
TListInfo		g_ListInfo[COLUMN_NUMBER];

HINSTANCE		g_hinst = NULL;		// handle to dll instance
tProcessDataProc g_ProcessDataProc;
tProcessDataProcW g_ProcessDataProcW;
HWND			g_MainWin;
char			g_pWorkingDir[PATH_LENGTH]="";
char			g_pCfgFileName[PATH_LENGTH]="CatalogMaker.ini";
