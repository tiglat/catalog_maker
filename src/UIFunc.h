/*****************************************************************************
**
**  FILE   :	UIFunc.h
**
**  PURPOSE:	Module contains user interface support functions
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#ifndef __UIFUNC_H__
#define __UIFUNC_H__

#include "windows.h"

void CheckButton( 
		HWND hDlg, 
		UINT ItemCode 
		);

void UncheckButton( 
		HWND hDlg, 
		UINT ItemCode 
		);

void EnableControl( 
		HWND hDlg, 
		UINT ItemCode 
		);

void DisableControl( 
		HWND hDlg, 
		UINT ItemCode 
		);

INT_PTR CALLBACK ChildDialogProc(
		HWND hChildDlg, 
		UINT message, 
		WPARAM wParam, 
		LPARAM lParam
		);

VOID OnSelChanged( 
		HWND hDlg 
		);

DLGTEMPLATE * DoLockDlgRes( 
		LPCTSTR lpszResName 
		);

void OnInitOptionDialog( 
		HWND hDlg 
		);

INT_PTR CALLBACK OptionsDialogProc(
		HWND hDlg, 
		UINT message, 
		WPARAM wParam, 
		LPARAM lParam
		);


#endif




