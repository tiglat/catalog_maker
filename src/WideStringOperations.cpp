#include "WideStringOperations.h"



bool WideStringOperations::IsNotNullOrEmpty(const WCHAR* pStr)
{
    return pStr != nullptr && wcslen(pStr) > 0;
}

size_t WideStringOperations::StrLen(const WCHAR* pStr)
{
    return wcslen(pStr);
}


const WCHAR* WideStringOperations::StrRChr(const WCHAR* pStr, const WCHAR chr)
{
    return wcsrchr(pStr, chr);
}

UINT WideStringOperations::StrNChr(const WCHAR* pStr, const WCHAR ch)
{
    UINT len = (UINT)wcslen(pStr);
    UINT Counter = 0;

    for (auto i = 0; i < len; i++)
    {
        if (pStr[i] == ch)
            Counter++;
    }

    return (Counter);
}


UINT WideStringOperations::StrCaseSensitiveCmp(const WCHAR* pLhsStr, const WCHAR* pRhsStr)
{
    return _wcsicmp(pLhsStr, pRhsStr);
}

