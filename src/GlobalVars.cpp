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
		{ TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, "*.*" };
TSortParam		g_SortParam = 
		{ TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };
TFormatParam	g_FormatParam = 
		{ TRUE, FALSE, 3, FALSE };

TDlgDesc		g_DlgDesc;		// 
TRxDesc			g_RxDesc;
TListInfo		g_ListInfo[COLUMN_NUMBER];

HINSTANCE		g_hinst = NULL;		// handle to dll instance
tProcessDataProc g_ProcessDataProc;
HWND			g_MainWin;
char			g_pWorkingDir[MAX_PATH]="";
char			g_pCfgFileName[MAX_PATH]="CatalogMaker.ini";
