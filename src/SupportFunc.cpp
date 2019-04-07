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

/*****************************************************************************
    Routine:     CalculateIndent
------------------------------------------------------------------------------
    Description: 
                Calculate Indent before File name if it is necessary
  
    Arguments:  
                pStr	- pointer to string for comparing

    Return Value:
                Value of indent


*****************************************************************************/

USHORT CalculateIndent( char *pStr )
{
    USHORT Indent = 0;

    if ( g_FormatParam.bIndentAll )
    {
        Indent = strnchr( pStr, '\\' );
        Indent = Indent * g_FormatParam.Width;
    }
    else
        if ( g_FormatParam.bIndentFiles && g_ViewParam.bFileName )
        {
            Indent = strnchr( pStr, '\\' );
            Indent = Indent ? g_FormatParam.Width : 0;
        }

    return Indent;
}

/*****************************************************************************
    Routine:     IsStrValid
------------------------------------------------------------------------------
    Description: 
                Compare pStr with pMask (wildcard)
  
    Arguments:  
                pStr		- pointer to string for comparing
                pMask	- pointer to string mask

    Return Value:
                Return TRUE if pStr matches pMask


*****************************************************************************/

BOOL IsStrValid( char *pStr, char *pMask )
{
    UINT IdxStr=0, IdxMsk=0;
    char *p;

    if ( pStr == pMask )
        return TRUE;

    if ( !pMask )
        return FALSE;

    if ( pStr )
    {
        if ( strlen(pStr) < strlen(pMask) )
            return FALSE;
    }

    while ( IdxMsk < strlen(pMask) )
    {
        if ( pMask[IdxMsk] == '*' )
        {
            IdxMsk++;
            if ( IdxMsk == strlen(pMask) ) 
                break;

            if ( pMask[IdxMsk] == '*' || pMask[IdxMsk] == '?' ) 
                continue;
            else
            {
                if ( !pStr ) 
                    return FALSE;

                p = strchr( &pStr[IdxStr+1], pMask[IdxMsk] );

                if ( p )
                    IdxStr = (UINT) (p - pStr);
                else
                    return FALSE;
            }
        }
        else
        {
            if ( pMask[IdxMsk] == '?' )
            {
                if ( IdxMsk == strlen(pMask) ) 
                    break;
            }
            else
            {
                if ( !pStr ) 
                    return FALSE;

                if ( pMask[IdxMsk] != pStr[IdxStr] )
                    return FALSE;
            }

            IdxMsk++;
            IdxStr++;
        }
    }

    return TRUE;
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

char *GetShortFileName( char *pFullName )
{
     char *pFileName = strrchr( pFullName, '\\' );

     if ( pFileName )
        pFileName++;
     else 
        pFileName = pFullName;

     return ( pFileName );
}

/*****************************************************************************
    Routine:     IsFileNameValid
------------------------------------------------------------------------------
    Description: 
                Test the file name with a mask (wildcard)
                The short file name is devided into a name and an extension
  
    Arguments:  
                pFileName	- pointer to short file name
                pMask		- pointer to mask

    Return Value:
                return TRUE if pFileName is matched to pMask


*****************************************************************************/

BOOL IsFileNameValid( char *pFileName, char *pMask )
{
    char *pExt, *pExtMask;

    // separate file name and extension in the original file name
    pExt = strrchr( pFileName, '.' );

    if ( pExt && pExt != pFileName )
    {
        *pExt = 0;
        pExt++;
    }

    // separate file name and extension in the mask
    pExtMask = strrchr( pMask, '.' );

    if ( pExtMask && pExtMask != pFileName )
    {
        *pExtMask = 0;
        pExtMask++;
    }

    // compare file name and extension with their masks
    BOOL F = 
        IsStrValid( pFileName, pMask ) && 
        IsStrValid( pExt, pExtMask );

    // restore previous values
    if ( pExt && pExt != pFileName )
    {
        pExt--;
        *pExt = '.';
    }

    if ( pExtMask && pExtMask != pFileName )
    {
        pExtMask--;
        *pExtMask = '.';
    }

    return ( F );
}

/*****************************************************************************
    Routine:     CompareFileNameWithMaskList
------------------------------------------------------------------------------
    Description: 
                Compare short file name with mask list
                The length of this list must be define by
                MASK_LIST_LENGTH constant and mask separator must be ';'
  
    Arguments:  
                pFileName	- pointer to short file name
                pMaskList	- pointer to mask list

    Return Value:
                return TRUE if pFileName is matched to on of the mask
                from mask list pMaskList

*****************************************************************************/

BOOL CompareFileNameWithMaskList( char *pFileName, char *pMaskList )
{
    char *pMask;
    char seps[] = ";";
    char FileMasks[MASK_LIST_LENGTH];
    char FileName[MAX_PATH];

    strcpy( FileMasks, pMaskList );
    strcpy( FileName, pFileName );
    _strlwr( FileName );
    _strlwr( FileMasks );
    
    pMask = strtok( FileMasks, seps );

    while( pMask != NULL )
    {
        if ( IsFileNameValid( FileName, pMask ) )
            return TRUE;
        
        pMask = strtok( NULL, seps );
    }

    return FALSE;
}

/*****************************************************************************
    Routine:     FormatIntNumber
------------------------------------------------------------------------------
    Description: 
                Convert Number to string and insert digit
                group separator ',' between thousands
  
    Arguments:  
                Number	- number for converting

    Return Value:
                pResultStr	- formated string
                Return length of the formated string

*****************************************************************************/

DWORD FormatIntNumber( char *pResultStr, DWORD64 Number ) 
{
    char	str[20], str1[20];
    char	*pstr, *pstr1;
    UINT	IntPart, RemainPart, length;

    pstr  = str;
    pstr1 = str1;

    memset( str, 0, 20 );
    memset( str1, 0, 20 );

    sprintf( str, "%I64d", Number );
    length = (UINT) strlen( str );

    if ( length <= 3 )
    {
        memcpy( pResultStr+15-length, str, length );
        return ( 15 );
    }

    IntPart		= length / 3;
    RemainPart	= length % 3;
    
    if ( RemainPart ) 
    {
        strncpy( pstr1, pstr, RemainPart );
        pstr = &pstr[RemainPart];
        str1[RemainPart] = ',';
        pstr1 = &pstr1[RemainPart+1]; 
    }

    for ( UINT i = 0; i < IntPart-1; i++ )
    {
        strncpy( pstr1, pstr, 3 );
        pstr = &pstr[3];
        pstr1[3] = ',';
        pstr1 = &pstr1[4]; 
    }
    
    strncpy( pstr1, pstr, 3 );

    length = (UINT) strlen( str1 );
    memcpy( pResultStr+15-length, str1, length );

    return ( 15 );
}

/*****************************************************************************
    Routine:     GetSlashNumber
------------------------------------------------------------------------------
    Description: 
                Count the number of "ch" symbols in the full file name

    Arguments:  
                pFileName	- full file name

    Return Value:
                Return number of "ch" symbols

*****************************************************************************/

UINT strnchr( char *pStr, char ch )
{
    UINT len = (UINT) strlen( pStr );
    UINT Counter = 0, i;

    for ( i = 0; i < len; i++ )
    {
        if ( pStr[i] == ch )
            Counter++;
    }

    return ( Counter );
}

/*****************************************************************************
    Routine:     IsFileSize 
------------------------------------------------------------------------------
    Description: 
                Determines if pStr is file size

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

BOOL IsFileSize ( char *pStr )
{
    UINT Len = (UINT) strlen( pStr ), i;
    BOOL Result = TRUE;

    if ( pStr == NULL )
        return ( FALSE );

    for ( i = 0; i< Len; i++ )
    {
        if ( !( iswdigit(pStr[i]) || pStr[i] == ',' ) )
        {
            Result = FALSE;
            break;
        }
    }

    return ( Result );
}

/*****************************************************************************
    Routine:     IsFileDate
------------------------------------------------------------------------------
    Description: 
                Determines if pStr is file date

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

BOOL IsFileDate ( char *pStr )
{
    BOOL Result = TRUE;

    if ( pStr == NULL )
        return ( FALSE );

    //if ( !strchr( pStr, '/' ) )	
    //	Result = FALSE;

    if ( !(iswdigit(pStr[0]) && 
        iswdigit(pStr[1]) &&
        iswdigit(pStr[3]) &&
        iswdigit(pStr[4]) &&
        iswdigit(pStr[6]) &&
        iswdigit(pStr[7]) && 
        pStr[2] == '.' &&
        pStr[5] == '.')
        )
        Result = FALSE;


    return ( Result );
}

/*****************************************************************************
    Routine:     IsFileTime 
------------------------------------------------------------------------------
    Description: 
                Determines if pStr is file time

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

BOOL IsFileTime ( char *pStr )
{
    BOOL Result = FALSE;

    if ( pStr == NULL )
        return ( Result );

    if ( strchr( pStr, ':' ) && !strchr( pStr, '\\' ) )	
        Result = TRUE;

    return ( Result );
}

/*****************************************************************************
    Routine:     IsFileAttr 
------------------------------------------------------------------------------
    Description: 
                Determines if pStr is file attributes

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

BOOL IsFileAttr ( char *pStr )
{
    BOOL Result = TRUE;

    if ( pStr == NULL )
        return ( FALSE );

    if ( strlen(pStr) == 4 )
    {
        for ( int i = 0; i < 4; i++ )
            if ( pStr[i] != 'r' && 
                pStr[i] != 'a' && 
                pStr[i] != 'h' &&
                pStr[i] != 's' &&
                pStr[i] != '-'
                )
            {
                Result = FALSE;
                break;
            }
    }

    else
        Result = FALSE;


    return ( Result );
}

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
Routine:     GetDirName
------------------------------------------------------------------------------
Description:
Extract directory name from a full directory name.

Arguments:
pFileInfo   - structure contained the name to parse

Return Value:

*****************************************************************************/

void GetShortDirName( TFileInfo *pFileInfo )
{
    char* pstr;

    if (pFileInfo == NULL)
        return;

    // Check if dir name is full
    pstr = strchr(pFileInfo->Name, ':');

    // If it is full, then extract the short dir name
    if (pstr)
    {
        // if file name is full then we need to cut root dir
        USHORT len = (USHORT)strlen(pFileInfo->Name);
        pFileInfo->Name[len - 1] = 0;
        pstr = strrchr(pFileInfo->Name, '\\');
        pFileInfo->Name[len - 1] = '\\';

        if (pstr)
        {
            g_RxDesc.RootDirLen = (USHORT) ( strlen(pFileInfo->Name) - strlen(++pstr) );
            strcpy(g_RxDesc.DirName, pstr);
        }
        else
        {
            strcpy(g_RxDesc.DirName, pFileInfo->Name);
        }

    }
    else
    {
        strcpy(g_RxDesc.DirName, pFileInfo->Name);
    }

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
    Routine:     GetFileName
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to number

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

void GetFileName( 
        char *pStr,
        USHORT StrLen,
        TFileInfo *pFileInfo )
{
    if ( pStr == NULL ) 
        return;

    int len = (int) strlen(pStr);

    // if pStr is wrong or pStr is directory 
    // than we should correct StrLen

    // we can read wrong file which hase sentence "File name" 
    // in the first line and at first position I this case StrLen may be wrong

    // if pStr is derictory we should correct length because dir name has 
    // longer length than column width It is correct situation
    if ( StrLen > len || pStr[len-2] == '\\' )
        StrLen = (USHORT) strlen( pStr );

    DelSpacesAroundStr( pFileInfo->Name, pStr, StrLen );

    if ( pFileInfo->Name[strlen(pFileInfo->Name)-1] == '\\' )
        pFileInfo->Attr |= 0x10;        

}

/*****************************************************************************
    Routine:     GetExt
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to number

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

void GetExt( 
        char *pStr,
        USHORT StrLen,
        TFileInfo *pFileInfo )
{
    char pExt[256];

    if ( pStr == NULL ) 
        return;

    int len = (int) strlen(pStr);

    if ( StrLen > len )
        StrLen = (USHORT) strlen( pStr );

    DelSpacesAroundStr( pExt, pStr, StrLen );

    // if file extension is not empty
    if ( strlen( pExt ) )
    {
        strcat( pFileInfo->Name, "." );
        strcat( pFileInfo->Name, pExt );
    }
}

/*****************************************************************************
    Routine:     GetSize 
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to number

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

void GetSize( 
        char *pStr,
        USHORT StrLen,
        TFileInfo *pFileInfo )
{
    char seps[] = ",";
    char *token, idx=0;
    char sSize[20], Source[20];

    if ( pStr == NULL ) 
        return;

    DelSpacesAroundStr( Source, pStr, StrLen);

    if ( !IsFileSize(Source) )
        return;
    
    token = strtok( Source, seps );

    while( token != NULL )
    {
        idx += sprintf( sSize+idx, "%s", token );
        token = strtok( NULL, seps );
    }

    pFileInfo->iSize = _atoi64( sSize );
    return;
}

/*****************************************************************************
    Routine:     GetDate 
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to date

    Arguments:  
                pStr		- pointer to string
                pFileInfo	- pointer to structure with file attributes


    Return Value:

*****************************************************************************/

void GetDate( 
        char *pStr,
        USHORT StrLen,
        TFileInfo *pFileInfo )
{
    char seps[] = ".";
    char *token;
    char Source[20];

    if ( pStr == NULL ) 
        return;
    
    memcpy( Source, pStr, StrLen );
    Source[StrLen] = 0;

    if ( !IsFileDate(Source) )
        return;

    token = strtok( Source, seps );

    if ( token )
    {
        pFileInfo->Day = atoi( token );
        token = strtok( NULL, seps );
    }

    if ( token )
    {
        pFileInfo->Month = atoi( token ) - 1;
        token = strtok( NULL, seps );
    }

    if ( token )
    {
        pFileInfo->Year = atoi( token ) - 1900;
        token = strtok( NULL, seps );
    }

    return;
}

/*****************************************************************************
    Routine:     GetTime 
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to time

    Arguments:  
                pStr		- pointer to string
                pFileInfo	- pointer to structure with file attributes

    Return Value:

*****************************************************************************/

void GetTime( 
        char *pStr,
        USHORT StrLen,
        TFileInfo *pFileInfo )
{
    char seps[] = ":.";
    char *token;
    char Source[20];

    if ( pStr == NULL ) 
        return;

    memcpy( Source, pStr, StrLen );
    Source[StrLen] = 0;

    if ( !IsFileTime(Source) )
        return;

    token = strtok( Source, seps );

    if ( token )
    {
        pFileInfo->Hour = atoi( token );
        token = strtok( NULL, seps );
    }

    if ( token )
    {
        pFileInfo->Minute = atoi( token );
        token = strtok( NULL, seps );
    }

    if ( token )
    {
        pFileInfo->Second = atoi( token );
    }

    return;
}

/*****************************************************************************
    Routine:     GetAttr 
------------------------------------------------------------------------------
    Description: 
                Convert pStr from string form to file attr

    Arguments:  
                pStr	- pointer to string

    Return Value:

*****************************************************************************/

void GetAttr( 
        char *pStr,
        USHORT StrLen,
        TFileInfo *pFileInfo )
{
    DWORD Result = 0;
    char Source[20];

    if ( pStr == NULL ) 
        return;

    // get value of attributes from the string
    memcpy( Source, pStr, StrLen );
    Source[StrLen] = 0;

    // test if the substring is attributes
    if ( !IsFileAttr(Source) )
        return;

    // analyse attr substring
    for( int i = 0; i< 4; i++ )
    {
        switch ( pStr[i] )
        {
            case 'a':
                Result |= 0x20;
                break;

            case 'h':
                Result |= 0x02;
                break;

            case 'r':
                Result |= 0x01;
                break;

            case 's':
                Result |= 0x04;
                break;
        }
    }

    pFileInfo->Attr |= Result;
    return;
}

/*****************************************************************************
    Routine:     FileInfoStringParser 
------------------------------------------------------------------------------
    Description: Analyse string from list file by using header information

    Arguments:  
            pStr	-	pointer to string for analyse
            pFileInfor	-	pointer to structure describing file in the string

    Return Value:
            None

*****************************************************************************/

void FileInfoStringParser( char *pStr, TFileInfo *pFileInfo )
{
    // we don't know what colomns exist so we have to check all descriptors
    for ( int i = 0; i < COLUMN_NUMBER; i++ )
    {
        // if the column is present than run the handler function
        if ( g_ListInfo[i].StartIdx /*|| i == COL_NAME*/ )
            g_ListInfo[i].HandleFunc( 
                &pStr[g_ListInfo[i].StartIdx-1], 
                g_ListInfo[i].Len, 
                pFileInfo
                );

        // if column is file name and last charachter is slash
        // than this is directory name and we can return
//		if ( i == COL_NAME && 
//			pFileInfo->Name[strlen(pFileInfo->Name)-1] == '\\')
//		{
//			pFileInfo->Attr |= 0x10;
//			return;
//		}
    }

    return;
}

/*****************************************************************************
    Routine:     HeaderInfoStringParser 
------------------------------------------------------------------------------
    Description:
                Analyse header of list file and fill array of column 
                descriptors which will use to read list file on the next steps

    Arguments:  
                pStr	-	pointer to string from list file

    Return Value:
                If this string matches list file format than TRUE 
                will be returned

*****************************************************************************/

BOOL HeaderInfoStringParser( char *pStr )
{
    char *Offset;

    if ( g_RxDesc.iThisIsHeader == 2 ) // first line of the header
    {
        //======= find and handle column "File name" ============
        Offset = strstr( pStr, "File name" );

        if ( Offset == NULL || Offset != pStr )
            return FALSE;

        g_ListInfo[COL_NAME].StartIdx = 1;
        g_ListInfo[COL_NAME].HandleFunc = &GetFileName;

        //======= find and handle column "Ext" ============
        Offset = strstr( pStr, "Ext" );

        g_ListInfo[COL_EXT].StartIdx = 
            Offset ? 
            (USHORT) ( Offset - pStr + 1) : 
            0;

        g_ListInfo[COL_EXT].HandleFunc = &GetExt;
        g_ListInfo[COL_NAME].Len = 
            Offset ?
            (USHORT) ( Offset - pStr ) :
            0;

        //======= find and handle column "Size" ============
        Offset = strstr( pStr, "Size" );

        g_ListInfo[COL_SIZE].StartIdx = 
            Offset ? 
            (USHORT) ( Offset - pStr + 1 ) : 
            0;

        g_ListInfo[COL_SIZE].HandleFunc = &GetSize;
        g_ListInfo[COL_SIZE].Len = 15;

        // if Size field exist and File name column width 
        // was not calculated yet
        if ( Offset && g_ListInfo[COL_NAME].Len == 0 )
        {
            g_ListInfo[COL_NAME].Len = (USHORT) ( Offset - pStr );
        }

        // if Size field exist and Ext column width 
        // was not calculated yet
        if ( Offset && g_ListInfo[COL_EXT].Len == 0 )
        {
            g_ListInfo[COL_EXT].Len = 
                (USHORT) ( Offset  - pStr - g_ListInfo[COL_EXT].StartIdx - 1 );
        }

        //======= find and handle column "Date" ============
        Offset = strstr( pStr, "Date" );

        g_ListInfo[COL_DATE].StartIdx = 
            Offset ? 
            (USHORT) ( Offset - pStr + 1 ) : 
            0;

        g_ListInfo[COL_DATE].HandleFunc = &GetDate;
        g_ListInfo[COL_DATE].Len = 10;

        if ( Offset && g_ListInfo[COL_NAME].Len == 0 )
        {
            g_ListInfo[COL_NAME].Len = (USHORT) ( Offset - pStr );
        }

        if ( Offset && g_ListInfo[COL_EXT].Len == 0 )
        {
            g_ListInfo[COL_EXT].Len = 
                (USHORT) ( Offset  - pStr - g_ListInfo[COL_EXT].StartIdx - 1 );
        }

        //======= find and handle column "Time" ============
        Offset = strstr( pStr, "Time" );

        g_ListInfo[COL_TIME].StartIdx = 
            Offset ? 
            (USHORT) ( Offset - pStr + 1 ) : 
            0;

        g_ListInfo[COL_TIME].HandleFunc = &GetTime;
        g_ListInfo[COL_TIME].Len = 8;

        if ( Offset && g_ListInfo[COL_NAME].Len == 0 )
        {
            g_ListInfo[COL_NAME].Len = (USHORT) ( Offset - pStr );
        }

        if ( Offset && g_ListInfo[COL_EXT].Len == 0 )
        {
            g_ListInfo[COL_EXT].Len = 
                (USHORT) ( Offset  - pStr - g_ListInfo[COL_EXT].StartIdx - 1 );
        }

        //======= find and handle column "Attr" ============
        Offset = strstr( pStr, "Attr" );

        g_ListInfo[COL_ATTR].StartIdx = 
            Offset ? 
            (USHORT) ( Offset - pStr + 1 ) : 
            0;

        g_ListInfo[COL_ATTR].HandleFunc = &GetAttr;
        g_ListInfo[COL_ATTR].Len = 4;

        g_ListInfo[COL_NAME].Len = 
            g_ListInfo[COL_NAME].Len ? 
            g_ListInfo[COL_NAME].Len : 
            ( Offset ? (USHORT) ( Offset - pStr ) : (USHORT) strlen( pStr ));

        g_ListInfo[COL_EXT].Len = 
            g_ListInfo[COL_EXT].Len ? 
            g_ListInfo[COL_EXT].Len : 
            (	Offset ? 
                (USHORT) ( Offset - pStr - g_ListInfo[COL_EXT].StartIdx - 1 ): 
                (USHORT) strlen( pStr ) - g_ListInfo[COL_EXT].StartIdx - 1 );
    }

    if ( g_RxDesc.iThisIsHeader == 1 ) // second and the last line of the header
    {
        // wrong file
        if ( g_ListInfo[COL_NAME].StartIdx == 0 )
            return FALSE;
    }

    g_RxDesc.iThisIsHeader--;

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

bool ReplaceAll(std::string &str, const std::string &from, const std::string &to) 
{
    size_t StartPos = str.find(from);
    
    if (StartPos == std::string::npos)
    {
        return false;
    }
    
    str.replace(StartPos, from.length(), to);
    return true;
}

void ConvertWildCardToRegex(std::string &pattern)
{
    ReplaceAll(pattern, "\\", "\\\\");
    ReplaceAll(pattern, "^", "\\^");
    ReplaceAll(pattern, ".", "\\.");
    ReplaceAll(pattern, "$", "\\$");
    ReplaceAll(pattern, "|", "\\|");
    ReplaceAll(pattern, "(", "\\(");
    ReplaceAll(pattern, ")", "\\)");
    ReplaceAll(pattern, "[", "\\[");
    ReplaceAll(pattern, "]", "\\]");
    ReplaceAll(pattern, "*", ".*");
    ReplaceAll(pattern, "+", "\\+");
    ReplaceAll(pattern, "?", ".");
    ReplaceAll(pattern, "/", "\\/");
}




