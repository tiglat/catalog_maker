#pragma once

#include "IStringOperations.h"

class AnsiStringOperations : public IStringOperations<char>
{
public:
    virtual bool IsNotNullOrEmpty(const char* pStr);
    virtual size_t StrLen(const char* pStr);
    virtual char* StrRChr(char* pStr, char ch);
    virtual UINT StrNChr(const char* pStr, const char ch);
    virtual UINT StrCaseSensitiveCmp(const char* pLhsStr, const char* pRhsStr);
};

