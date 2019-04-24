#pragma once

#include <string>
#include "IStringOperations.h"

class AnsiStringOperations : public IStringOperations<char, std::string>
{
public:
    virtual bool IsNotNullOrEmpty(const char* pStr);
    virtual size_t StrLen(const char* pStr);
    virtual char* StrRChr(char* pStr, char ch);
    virtual UINT StrNChr(const char* pStr, const char ch);
    virtual INT StrCaseInsensitiveCmp(const char* pLhsStr, const char* pRhsStr);
    virtual std::string ConvertDateToString(SYSTEMTIME& DateTime);
    virtual std::string ConvertTimeToString(SYSTEMTIME& DateTime);
    virtual std::string ConvertIntToString(DWORD64 num);
    virtual std::string GetEndLineChars();
};

