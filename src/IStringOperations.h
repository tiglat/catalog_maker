#pragma once

#include "windows.h"

template <typename TChar, typename TString> class IStringOperations
{
public:
    virtual bool IsNotNullOrEmpty(const TChar* pStr) = 0;
    virtual size_t StrLen(const TChar* pStr) = 0;
    virtual TChar* StrChr(TChar* pStr, TChar ch) = 0;
    virtual TChar* StrRChr(TChar* pStr, TChar ch) = 0;

    /*****************************************************************************
        Routine:     StrNChr
    ------------------------------------------------------------------------------
        Description:
                    Count the number of "ch" symbols in the string

        Arguments:
                    pStr	- string to analyze
                    ch      - symbol to count

        Return Value:
                    Return number of "ch" symbols

    *****************************************************************************/
    
    virtual UINT StrNChr(const TChar* pStr, const TChar ch) = 0;


    virtual INT StrCaseInsensitiveCmp(const TChar* pLhsStr, const TChar* pRhsStr) = 0;

    virtual TChar* StrTok(TChar* strToken, const TChar* strDelimit) = 0;
    virtual TChar* StrCpy(TChar* destination, const TChar* source) = 0;
    virtual TChar* StrCat(TChar* destination, const TChar* source) = 0;
    virtual const TChar* StrStr(const TChar* destination, const TChar* source) = 0;

    virtual TString ConvertDateToString(SYSTEMTIME& DateTime) = 0;
    virtual TString ConvertTimeToString(SYSTEMTIME& DateTime) = 0;
    virtual TString ConvertIntToString(DWORD64 num) = 0;

    virtual TString GetEndLineChars() = 0;

    virtual TChar* GetFileNameColumnPtr(const TChar* pStr) = 0;
    virtual TChar* GetExtColumnPtr(const TChar* pStr) = 0;
    virtual TChar* GetSizeColumnPtr(const TChar* pStr) = 0;
    virtual TChar* GetDateColumnPtr(const TChar* pStr) = 0;
    virtual TChar* GetTimeColumnPtr(const TChar* pStr) = 0;
    virtual TChar* GetAttrColumnPtr(const TChar* pStr) = 0;

};

