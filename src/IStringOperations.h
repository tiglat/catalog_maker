#pragma once

#include "windows.h"

template <typename TChar> class IStringOperations
{
public:
    virtual bool IsNotNullOrEmpty(const TChar* pStr) = 0;
    virtual size_t StrLen(const TChar* pStr) = 0;
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


    virtual UINT StrCaseSensitiveCmp(const TChar* pLhsStr, const TChar* pRhsStr) = 0;
};

