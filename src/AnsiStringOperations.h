#pragma once

#include <string>
#include "IStringOperations.h"

class AnsiStringOperations : public IStringOperations<char, std::string>
{
public:

    virtual bool IsNotNullOrEmpty(const char* pStr);

    virtual size_t StrLen(const char* pStr);
    virtual char* StrChr(char* pStr, char ch);
    virtual char* StrRChr(char* pStr, char ch);
    virtual UINT StrNChr(const char* pStr, const char ch);
    virtual INT StrCaseInsensitiveCmp(const char* pLhsStr, const char* pRhsStr);
    virtual char* StrTok(char* strToken, const char* strDelimit);
    virtual char* StrCpy(char* destination, const char* source);
    virtual char* StrCat(char* destination, const char* source);
    virtual char* StrStr(const char* destination, const char* source);

    virtual char* StrNCpy(char* destination, const char* source, UINT len);
    virtual char* StrNCat(char* destination, const char* source, UINT len);

    virtual std::string ConvertDateToString(SYSTEMTIME& DateTime);
    virtual std::string ConvertTimeToString(SYSTEMTIME& DateTime);
    virtual std::string ConvertFileSizeToString(DWORD64 num);
    virtual std::string ConvertIntToString(DWORD num);
    virtual DWORD64 ConvertStringToInt(std::string& Str);
    virtual DWORD64 ConvertStringToInt(const char* Str);

    virtual std::string GetEndLineChars();

};

