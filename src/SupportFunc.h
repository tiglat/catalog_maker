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

void DelSpacesAroundStr ( char *pResultStr, char *pStr, USHORT Len );
BOOL ReadConfigData();
BOOL RemoveFileName(char *pFullPath);
void ConvertWildCardToRegexA(std::string& pattern);
void ConvertWildCardToRegexW(std::wstring& pattern);

template <typename TString> bool ReplaceAll(TString &str, const TString &from, const TString &to)
{
    size_t StartPos = str.find(from);

    if (StartPos == TString::npos)
    {
        return false;
    }

    str.replace(StartPos, from.length(), to);
    return true;
}




#endif


