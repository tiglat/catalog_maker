/*****************************************************************************
**
**  FILE   :	SupportFunc.cpp
**
**  PURPOSE:	
**
**	Author:		Konstantin Polyakov
**
*****************************************************************************/

#include "SupportFunc.h"

#include "stdio.h"
#include "locale.h"
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <regex>

using namespace std;


/*****************************************************************************
    Routine:     DelSpacesAroundStr
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to number

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

void DelSpacesAroundStr ( char *pResultStr, char *pStr, USHORT Len )
{
    USHORT i = Len, StrLen = Len;

    // count postfix spaces
    while ( pStr[i-1] == 32 || pStr[i-1] == 0x0D )
    {
        if ( i == 0 )
            break;
        i--;
    }
    
    StrLen = i;

    i = 0;

    // count prefix spaces
    while ( pStr[i] == 32 && i < StrLen )
    {
        i++;
    }

    StrLen -= i;

    // get substring between spaces
    memcpy( pResultStr, &pStr[i], StrLen );
    pResultStr[StrLen] = 0;
}

/*****************************************************************************
Routine:     RemoveFileName
------------------------------------------------------------------------------
Description:
Exclude file name from full path to get just a path

Arguments:
pFullPath   - fully qualified path for a file [INPUT]

Return Value:
    result value
    non zero - success
*****************************************************************************/

BOOL RemoveFileName(char *pFullPath)
{
    //
    char *pFileName = strrchr(pFullPath, '\\');
    
    if (pFileName == NULL) 
        return FALSE;
    
    pFileName[0] = 0;
    return TRUE;
}


/*****************************************************************************
    Routine:     ReadConfigData
------------------------------------------------------------------------------
    Description:
                Reads configuration data from ini file. 

    Arguments:  
                

    Return Value:
                TRUE - success 

*****************************************************************************/

BOOL ReadConfigData()
{
    SetCurrentDirectory( g_pWorkingDir );
    FILE *OptionsFile = fopen( g_pCfgFileName, "r" );

    if ( OptionsFile )
    {
        fread( &g_ViewParam, sizeof(g_ViewParam), 1, OptionsFile );
        if( ferror( OptionsFile ) ) 
        {
            fclose( OptionsFile );
            return FALSE;
        }

        fread( &g_SortParam, sizeof(g_SortParam), 1, OptionsFile );
        if( ferror( OptionsFile ) ) 
        {
            fclose( OptionsFile );
            return FALSE;
        }

        fread( &g_FormatParam, sizeof(g_FormatParam), 1, OptionsFile );
        if( ferror( OptionsFile ) ) 
        {
            fclose( OptionsFile );
            return FALSE;
        }

        fclose( OptionsFile );
    }
    return TRUE;
}

/*****************************************************************************
    Routine:     ConvertWildCardToRegexA
------------------------------------------------------------------------------
    Description:
                Convert original ANSI string from wild card to regular expression.

    Arguments:
                pattern - [in][out] string to be converted

    Return Value:
                nothing

*****************************************************************************/

void ConvertWildCardToRegexA(string& pattern)
{
    ReplaceAll<string>(pattern, "\\", "\\\\");
    ReplaceAll<string>(pattern, "^", "\\^");
    ReplaceAll<string>(pattern, ".", "\\.");
    ReplaceAll<string>(pattern, "$", "\\$");
    ReplaceAll<string>(pattern, "|", "\\|");
    ReplaceAll<string>(pattern, "(", "\\(");
    ReplaceAll<string>(pattern, ")", "\\)");
    ReplaceAll<string>(pattern, "[", "\\[");
    ReplaceAll<string>(pattern, "]", "\\]");
    ReplaceAll<string>(pattern, "*", ".*");
    ReplaceAll<string>(pattern, "+", "\\+");
    ReplaceAll<string>(pattern, "?", ".");
    ReplaceAll<string>(pattern, "/", "\\/");
}

/*****************************************************************************
    Routine:     ConvertWildCardToRegexW
------------------------------------------------------------------------------
    Description:
                Convert original Unicode string from wild card to regular expression.

    Arguments:
                pattern - [in][out] string to be converted

    Return Value:
                nothing

*****************************************************************************/

void ConvertWildCardToRegexW(wstring& pattern)
{
    ReplaceAll<wstring>(pattern, L"\\", L"\\\\");
    ReplaceAll<wstring>(pattern, L"^", L"\\^");
    ReplaceAll<wstring>(pattern, L".", L"\\.");
    ReplaceAll<wstring>(pattern, L"$", L"\\$");
    ReplaceAll<wstring>(pattern, L"|", L"\\|");
    ReplaceAll<wstring>(pattern, L"(", L"\\(");
    ReplaceAll<wstring>(pattern, L")", L"\\)");
    ReplaceAll<wstring>(pattern, L"[", L"\\[");
    ReplaceAll<wstring>(pattern, L"]", L"\\]");
    ReplaceAll<wstring>(pattern, L"*", L".*");
    ReplaceAll<wstring>(pattern, L"+", L"\\+");
    ReplaceAll<wstring>(pattern, L"?", L".");
    ReplaceAll<wstring>(pattern, L"/", L"\\/");
}                                




