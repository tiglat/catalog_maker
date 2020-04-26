/*****************************************************************************
**
**  FILE   :	UIFunc.cpp
**
**  PURPOSE:	Module contains user interface support functions
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#include "GlobalVars.h"
#include "UIFunc.h"
#include "SupportFunc.h"
#include "commctrl.h"
#include "resource.h" 
#include "stdio.h" 


/*****************************************************************************
    Routine:     CheckButton
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

void CheckButton( HWND hDlg, UINT ItemCode )
{
    HWND hItem =  GetDlgItem(hDlg, ItemCode);
    SendMessage(hItem, BM_SETCHECK, BST_CHECKED, 0);
}

/*****************************************************************************
    Routine:     UncheckButton
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

void UncheckButton( HWND hDlg, UINT ItemCode )
{
    HWND hItem =  GetDlgItem(hDlg, ItemCode);
    SendMessage(hItem, BM_SETCHECK, BST_UNCHECKED, 0);
}

/*****************************************************************************
    Routine:     EnableControl
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

void EnableControl( HWND hDlg, UINT ItemCode )
{
    HWND hItem =  GetDlgItem(hDlg, ItemCode);
    EnableWindow( hItem, TRUE );
}

/*****************************************************************************
    Routine:     EnableControl
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

void DisableControl( HWND hDlg, UINT ItemCode )
{
    HWND hItem =  GetDlgItem(hDlg, ItemCode);
    EnableWindow( hItem, FALSE );
}

/*****************************************************************************
    Routine:     ChildDialogProc
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

INT_PTR CALLBACK ChildDialogProc(
            HWND hChildDlg, 
            UINT message, 
            WPARAM wParam, 
            LPARAM lParam
            )
{
    switch ( message )
    {
        case WM_INITDIALOG:
        {
            RECT rcTab, rc;

            GetWindowRect(g_DlgDesc.hTabCtrl, &rcTab);
            TabCtrl_AdjustRect(g_DlgDesc.hTabCtrl, FALSE, &rcTab);
            CopyRect(&rc, &rcTab);
            OffsetRect(&rc, -rc.left, -rc.top);

            SetWindowPos(
                hChildDlg,
                HWND_TOP,
                rcTab.left,
                rcTab.top,
                rc.right,
                rc.bottom,
                0
            );

            return TRUE;
        }

        case WM_COMMAND:
        {
            LRESULT	rv, rv2;
            HWND	hItem, hItem2;

            switch (wParam)
            {
            case IDC_CHK_DIRECTORIES:

                hItem = GetDlgItem(hChildDlg, IDC_CHK_DIRECTORIES);
                rv = SendMessage(hItem, BM_GETCHECK, 0, 0);

                hItem2 = GetDlgItem(hChildDlg, IDC_CHK_APPLY_TO_DIRS);
                rv2 = SendMessage(hItem2, BM_GETCHECK, 0, 0);

                if (rv == BST_UNCHECKED)
                {
                    CheckButton(
                        g_DlgDesc.hChildDlg[0],
                        IDC_CHK_FILE_NAME
                    );

                    DisableControl(hChildDlg, IDC_CHECK_DIR_SIZE);

                    UncheckButton(
                        g_DlgDesc.hChildDlg[0],
                        IDC_CHECK_DIR_SIZE
                    );

                    DisableControl(hChildDlg, IDC_CHK_APPLY_TO_DIRS);

                    UncheckButton(
                        g_DlgDesc.hChildDlg[0],
                        IDC_CHK_APPLY_TO_DIRS
                    );
                }
                else
                {
                    EnableControl(hChildDlg, IDC_CHK_APPLY_TO_DIRS);
                }

            case IDC_CHK_APPLY_TO_DIRS:

                hItem = GetDlgItem(hChildDlg, IDC_CHK_FILE_NAME);
                rv = SendMessage(hItem, BM_GETCHECK, 0, 0);

                hItem2 = GetDlgItem(hChildDlg, IDC_CHK_APPLY_TO_DIRS);
                rv2 = SendMessage(hItem2, BM_GETCHECK, 0, 0);

                if (rv == BST_UNCHECKED && rv2 == BST_UNCHECKED)
                {
                    DisableControl(hChildDlg, IDC_CHK_FULL_NAME);
                    DisableControl(hChildDlg, IDC_CHK_SIZE);
                    DisableControl(hChildDlg, IDC_CHK_DATE);
                    DisableControl(hChildDlg, IDC_CHK_TIME);
                    DisableControl(hChildDlg, IDC_CHK_EXT);
                    DisableControl(hChildDlg, IDC_CHK_ATTR);
                    DisableControl(hChildDlg, IDC_ED_FILE_TYPES);
                    DisableControl(hChildDlg, IDC_CHECK_DIR_SIZE);
                }
                else if (rv2 == BST_CHECKED)
                {
                    EnableControl(hChildDlg, IDC_CHK_FULL_NAME);
                    EnableControl(hChildDlg, IDC_CHK_DATE);
                    EnableControl(hChildDlg, IDC_CHK_TIME);
                    EnableControl(hChildDlg, IDC_CHK_ATTR);
                    EnableControl(hChildDlg, IDC_CHECK_DIR_SIZE);
                }

                if (rv2 == BST_UNCHECKED)
                { 
                    DisableControl(hChildDlg, IDC_CHECK_DIR_SIZE);
                }

            case IDC_CHK_FILE_NAME:

                hItem = GetDlgItem(hChildDlg, IDC_CHK_FILE_NAME);
                rv = SendMessage(hItem, BM_GETCHECK, 0, 0);

                hItem2 = GetDlgItem(hChildDlg, IDC_CHK_APPLY_TO_DIRS);
                rv2 = SendMessage(hItem2, BM_GETCHECK, 0, 0);

                if (rv == BST_UNCHECKED)
                {
                    DisableControl(hChildDlg, IDC_ED_FILE_TYPES);
                    DisableControl(hChildDlg, IDC_CHK_SIZE);
                    DisableControl(hChildDlg, IDC_CHK_EXT);

                    if (rv2 == BST_UNCHECKED)
                    {
                        DisableControl(hChildDlg, IDC_CHK_FULL_NAME);
                        DisableControl(hChildDlg, IDC_CHK_DATE);
                        DisableControl(hChildDlg, IDC_CHK_TIME);
                        DisableControl(hChildDlg, IDC_CHK_ATTR);
                        DisableControl(hChildDlg, IDC_CHECK_DIR_SIZE);
                    }

                    CheckButton(
                        g_DlgDesc.hChildDlg[0],
                        IDC_CHK_DIRECTORIES
                    );

                    EnableControl(hChildDlg, IDC_CHK_APPLY_TO_DIRS);
                }
                else if (rv == BST_CHECKED)
                {
                    EnableControl(hChildDlg, IDC_CHK_FULL_NAME);
                    EnableControl(hChildDlg, IDC_CHK_SIZE);
                    EnableControl(hChildDlg, IDC_CHK_DATE);
                    EnableControl(hChildDlg, IDC_CHK_TIME);
                    EnableControl(hChildDlg, IDC_CHK_EXT);
                    EnableControl(hChildDlg, IDC_CHK_ATTR);
                    EnableControl(hChildDlg, IDC_ED_FILE_TYPES);
                }
                break;

            }
            return TRUE;
        }

        case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->code)
            {

                case NM_CLICK:          // Fall through to the next case.

                case NM_RETURN:
                {
                    PNMLINK pNMLink = (PNMLINK)lParam;
                    LITEM   item = pNMLink->item;
                    HWND hItem = GetDlgItem(hChildDlg, IDC_SYSLINK2);

                    if ((((LPNMHDR)lParam)->hwndFrom == hItem) && (item.iLink == 0 || item.iLink == 1))
                    {
                        ShellExecuteW(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
                    }

                    break;
                }
            }

            return TRUE;
        }
    }

    return FALSE;
}

/*****************************************************************************
    Routine:     OnSelChanged
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

VOID OnSelChanged( HWND hDlg ) 
{ 
    UINT CurPageIndex = TabCtrl_GetCurSel( g_DlgDesc.hTabCtrl ); 

    if (g_DlgDesc.hCurDlg != NULL)
    {
        ShowWindow(g_DlgDesc.hCurDlg, SW_HIDE);
    }
 
    // Create the new child dialog box. 
    g_DlgDesc.hCurDlg = g_DlgDesc.hChildDlg[CurPageIndex];
    ShowWindow( g_DlgDesc.hCurDlg, SW_SHOW );

} 

/*****************************************************************************
    Routine:     DoLockDlgRes
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

DLGTEMPLATE * DoLockDlgRes( LPCTSTR lpszResName ) 
{ 
    HRSRC hrsrc		= FindResource( g_hinst, lpszResName, RT_DIALOG ); 
    HGLOBAL hglb	= LoadResource( g_hinst, hrsrc ); 

    return (DLGTEMPLATE *) LockResource( hglb ); 
}

/*****************************************************************************
    Routine:     OnInitOptionDialog
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

void OnInitOptionDialog( HWND hDlg )
{
    TCITEM	Attributes = {0}; 

    // Create Tab Control
    g_DlgDesc.hTabCtrl = GetDlgItem( hDlg, IDC_TAB1 );

    Attributes.mask = TCIF_TEXT | TCIF_IMAGE;
    Attributes.iImage = -1; 
    Attributes.pszText = "View";
    TabCtrl_InsertItem( g_DlgDesc.hTabCtrl, 0, &Attributes ); 
    Attributes.pszText = "Sort"; 
    TabCtrl_InsertItem( g_DlgDesc.hTabCtrl, 1, &Attributes ); 
    Attributes.pszText = "Format"; 
    TabCtrl_InsertItem( g_DlgDesc.hTabCtrl, 2, &Attributes ); 
    Attributes.pszText = "About"; 
    TabCtrl_InsertItem( g_DlgDesc.hTabCtrl, 3, &Attributes ); 

    // load and create dialogs
    for ( int i = 0; i < OPTION_PAGES; i++ )
    {
        g_DlgDesc.ChildDlgRes[i] = 
            DoLockDlgRes( MAKEINTRESOURCE( IDD_VIEWDLG+i ) ); 

        // Create the new child dialog box. 
        g_DlgDesc.hCurDlg = 
        g_DlgDesc.hChildDlg[i] =
            CreateDialogIndirect(
                    g_hinst,
                    g_DlgDesc.ChildDlgRes[i],
                    hDlg,
                    ChildDialogProc
                    );
    }

    // setup default parameters

    // --------- for VIEW dialog --------------
    
    if (g_ViewParam.bDirName)
    {
        CheckButton(g_DlgDesc.hChildDlg[0], IDC_CHK_DIRECTORIES);
    }
    else
    {
        HWND hChildDlg = g_DlgDesc.hChildDlg[0];
        DisableControl(hChildDlg, IDC_CHECK_DIR_SIZE);
    }

    if ( g_ViewParam.bApplyToDirs )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_APPLY_TO_DIRS );

    if ( g_ViewParam.bFileName )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_FILE_NAME );
    else
    {
        HWND hChildDlg = g_DlgDesc.hChildDlg[0];

        if ( !g_ViewParam.bApplyToDirs )
        {
            DisableControl( hChildDlg, IDC_CHK_FULL_NAME );
            DisableControl( hChildDlg, IDC_CHK_DATE );
            DisableControl( hChildDlg, IDC_CHK_TIME );
            DisableControl( hChildDlg, IDC_CHK_ATTR );
        }

        DisableControl( hChildDlg, IDC_CHK_SIZE );
        DisableControl( hChildDlg, IDC_CHK_EXT );
        DisableControl( hChildDlg, IDC_ED_FILE_TYPES );
    }

    if ( g_ViewParam.bFullName )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_FULL_NAME );

    if ( g_ViewParam.bExt )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_EXT );

    if ( g_ViewParam.bSize )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_SIZE );

    if ( g_ViewParam.bDate )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_DATE );

    if ( g_ViewParam.bTime )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_TIME );

    if ( g_ViewParam.bAttr )
        CheckButton( g_DlgDesc.hChildDlg[0], IDC_CHK_ATTR );

    if (g_ViewParam.bDirSize)
        CheckButton(g_DlgDesc.hChildDlg[0], IDC_CHECK_DIR_SIZE);

    HWND hItem =  GetDlgItem(
                g_DlgDesc.hChildDlg[0], 
                IDC_ED_FILE_TYPES
                );

    DelSpacesAroundStr( 
        g_ViewParam.sFileTypes, 
        g_ViewParam.sFileTypes,
        (USHORT)strlen(g_ViewParam.sFileTypes) 
        );

    if ( strlen(g_ViewParam.sFileTypes) == 0 )
        strcpy( g_ViewParam.sFileTypes, "*.*" );

    SendMessage(  
        hItem, 
        WM_SETTEXT,
        sizeof(g_ViewParam.sFileTypes), 
        (LPARAM)g_ViewParam.sFileTypes
        );

    // --------- for SORT dialog --------------

    if ( g_SortParam.bName )
        CheckButton( g_DlgDesc.hChildDlg[1], IDC_RB_SORT_NAME );

    if ( g_SortParam.bExt )
        CheckButton( g_DlgDesc.hChildDlg[1], IDC_RB_SORT_EXT );

    if ( g_SortParam.bSize )
        CheckButton( g_DlgDesc.hChildDlg[1], IDC_RB_SORT_SIZE );

    if ( g_SortParam.bDate )
        CheckButton( g_DlgDesc.hChildDlg[1], IDC_RB_SORT_DATE );

    if ( g_SortParam.bUnsorted )
        CheckButton( g_DlgDesc.hChildDlg[1], IDC_RB_SORT_UNSORTED );

    if ( g_SortParam.bDescent )
        CheckButton( g_DlgDesc.hChildDlg[1], IDC_CHK_SORT_DESC );


    // --------- for FORMAT dialog --------------

    if ( g_FormatParam.bIndentFiles )
        CheckButton( g_DlgDesc.hChildDlg[2], IDC_CHK_FORMAT_FILES );

    if ( g_FormatParam.bIndentAll )
        CheckButton( g_DlgDesc.hChildDlg[2], IDC_CHK_FORMAT_ALL );

    if ( g_FormatParam.bExtSeparately )
        CheckButton( g_DlgDesc.hChildDlg[2], IDC_CHK_FORMAT_EXT );

    hItem =  GetDlgItem( 
                g_DlgDesc.hChildDlg[2], 
                IDC_ED_FORMAT_WIDTH
                );

    char Width[10];

    _itoa( g_FormatParam.Width, Width, 10 );

    SendMessage(  
        hItem, 
        WM_SETTEXT,
        (WPARAM) sizeof( Width ), 
        (LPARAM) Width
        );
}

/*****************************************************************************
    Routine:     GetOptionsFromWindow
------------------------------------------------------------------------------
    Description: Read options from the options dialog into global variables
  
    Arguments:  

    Return Value:


*****************************************************************************/

void GetOptionsFromWindow()
{

    // -------- View Dialog -----------------

    HWND hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_DIRECTORIES
                );

    LRESULT rv = 
        SendMessage( hItem, BM_GETCHECK, 0,	0 );

    g_ViewParam.bDirName = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_FILE_NAME
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0,	0 );

    g_ViewParam.bFileName = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem(
        g_DlgDesc.hChildDlg[0],
        IDC_CHECK_DIR_SIZE
    );

    rv = SendMessage(hItem, BM_GETCHECK, 0, 0);

    g_ViewParam.bDirSize =
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_APPLY_TO_DIRS 
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bApplyToDirs = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_APPLY_TO_DIRS
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bApplyToDirs = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_FULL_NAME
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bFullName = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_EXT
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bExt = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_SIZE
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bSize = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_DATE
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bDate = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_TIME
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bTime = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_CHK_ATTR
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_ViewParam.bAttr = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem =  GetDlgItem( 
                g_DlgDesc.hChildDlg[0], 
                IDC_ED_FILE_TYPES
                );

    SendMessage(  
        hItem, 
        WM_GETTEXT,
        (WPARAM)sizeof(g_ViewParam.sFileTypes), 
        (LPARAM)g_ViewParam.sFileTypes
        );

    DelSpacesAroundStr( 
        g_ViewParam.sFileTypes, 
        g_ViewParam.sFileTypes,
        (USHORT)strlen(g_ViewParam.sFileTypes)
        );

    if ( strlen(g_ViewParam.sFileTypes) == 0 )
        strcpy( g_ViewParam.sFileTypes, "*.*" );

    // -------- Sort Dialog -----------------

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[1], 
                IDC_RB_SORT_NAME
                );

    rv = 
        SendMessage( hItem, BM_GETCHECK, 0,	0 );

    g_SortParam.bName = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[1], 
                IDC_RB_SORT_EXT
                );

    rv = 
        SendMessage( hItem, BM_GETCHECK, 0,	0 );

    g_SortParam.bExt = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[1], 
                IDC_RB_SORT_SIZE
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_SortParam.bSize = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[1], 
                IDC_RB_SORT_DATE
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_SortParam.bDate = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[1], 
                IDC_RB_SORT_UNSORTED
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_SortParam.bUnsorted = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[1], 
                IDC_CHK_SORT_DESC
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_SortParam.bDescent = 
        rv == BST_CHECKED ? TRUE : FALSE;

    // -------- Format Dialog -----------------

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[2], 
                IDC_CHK_FORMAT_FILES
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_FormatParam.bIndentFiles = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[2], 
                IDC_CHK_FORMAT_ALL
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_FormatParam.bIndentAll = 
        rv == BST_CHECKED ? TRUE : FALSE;

    ///////////////////////////////////////

    hItem =  GetDlgItem( 
                g_DlgDesc.hChildDlg[2], 
                IDC_ED_FORMAT_WIDTH
                );

    char sWidth[10];

    SendMessage(  
        hItem, 
        WM_GETTEXT,
        (WPARAM)sizeof(sWidth), 
        (LPARAM)sWidth
        );

    g_FormatParam.Width = atoi( sWidth );

    ///////////////////////////////////////

    hItem = GetDlgItem( 
                g_DlgDesc.hChildDlg[2], 
                IDC_CHK_FORMAT_EXT
                );

    rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

    g_FormatParam.bExtSeparately = 
        rv == BST_CHECKED ? TRUE : FALSE;

}

/*****************************************************************************
    Routine:     OptionsDialogProc
------------------------------------------------------------------------------
    Description: 
  
    Arguments:  

    Return Value:


*****************************************************************************/

INT_PTR CALLBACK OptionsDialogProc(
            HWND hDlg, 
            UINT message, 
            WPARAM wParam, 
            LPARAM lParam
            )
{
    switch ( message )
    {
        case WM_INITDIALOG:
        {
            if ( !ReadConfigData() )
            {
                MessageBox(
                    NULL,
                    "Configuration data read error. Default settings will be used.",
                    "Warning",
                    MB_OK
                );
            }

            g_MainWin = hDlg;
            OnInitOptionDialog( hDlg );
            OnSelChanged( hDlg );
            return TRUE;
        }

        case WM_MOVE:

            for ( int i = 0; i < OPTION_PAGES; i++ )
            {
                SendMessage( g_DlgDesc.hChildDlg[i], WM_INITDIALOG, 0, 0 );
            }

            SendMessage( g_MainWin, WM_SETFOCUS, 0, 0 );
            break;

        case WM_NOTIFY:
            
            NMHDR *p;
            p = (NMHDR*) lParam;

            switch ( p->code ) 
            { 
                case TCN_SELCHANGE:

                        OnSelChanged( hDlg );
                        SendMessage( g_MainWin, WM_SETFOCUS, 0, 0 );
                        return TRUE;
            } 
            break; 

        case WM_COMMAND:
        
            switch ( wParam )
            {
                case IDOK:
                {
                    GetOptionsFromWindow();
                    SetCurrentDirectory( g_pWorkingDir );

                    FILE *OptionsFile = fopen( g_pCfgFileName, "w" );

                    if ( OptionsFile )
                    {
                        fwrite( 
                            &g_ViewParam, 
                            sizeof(g_ViewParam), 
                            1, 
                            OptionsFile 
                        );

                        if( ferror( OptionsFile ) ) 
                        {
                            fclose( OptionsFile );
                            return FALSE;
                        }

                        fwrite( 
                            &g_SortParam, 
                            sizeof(g_SortParam), 
                            1, 
                            OptionsFile 
                        );

                        if( ferror( OptionsFile ) ) 
                        {
                            fclose( OptionsFile );
                            return FALSE;
                        }

                        fwrite( 
                            &g_FormatParam, 
                            sizeof(g_FormatParam), 
                            1, 
                            OptionsFile 
                        );

                        if( ferror( OptionsFile ) ) 
                        {
                            fclose( OptionsFile );
                            return FALSE;
                        }

                        fclose( OptionsFile );
                    }
                    else
                    {
                        MessageBox(
                            NULL,
                            "Impossible to write options on your hard disk",
                            "Warning",
                            MB_OK
                        );
                    }
                }

                case IDCANCEL:

                    EndDialog( hDlg, wParam );
                    return TRUE;

            }
            break;
    }

    return FALSE;
}
