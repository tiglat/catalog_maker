#include "AnsiStringOperations.h"


bool AnsiStringOperations::IsNotNullOrEmpty(const char* pStr)
{
    return pStr != nullptr && strlen(pStr) > 0;
}

size_t AnsiStringOperations::StrLen(const char* pStr)
{
    return strlen(pStr);
}


char* AnsiStringOperations::StrRChr(char* pStr, char chr)
{
    return strrchr(pStr, chr);
}

UINT AnsiStringOperations::StrNChr(const char* pStr, const char ch)
{
    UINT len = (UINT)strlen(pStr);
    UINT Counter = 0;

    for (auto i = 0; i < len; i++)
    {
        if (pStr[i] == ch)
            Counter++;
    }

    return (Counter);
}


INT AnsiStringOperations::StrCaseInsensitiveCmp(const char* pLhsStr, const char* pRhsStr)
{
    return _stricmp(pLhsStr, pRhsStr);
}
