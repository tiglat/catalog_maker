#pragma once

#include "windows.h"
#include <string>
#include "IStringOperations.h"

class WideStringOperations : public IStringOperations<WCHAR, std::wstring>
{
public:
    virtual bool IsNotNullOrEmpty(const WCHAR* pStr);
    virtual size_t StrLen(const WCHAR* pStr);
    virtual WCHAR* StrChr(WCHAR* pStr, const WCHAR ch);
    virtual WCHAR* StrRChr(WCHAR* pStr, const WCHAR ch);
    virtual UINT StrNChr(const WCHAR* pStr, const WCHAR ch);
    virtual INT StrCaseInsensitiveCmp(const WCHAR* pLhsStr, const WCHAR* pRhsStr);
    virtual WCHAR* StrTok(WCHAR* strToken, const WCHAR* strDelimit);
    virtual WCHAR* StrCpy(WCHAR* destination, const WCHAR* source);
    virtual WCHAR* StrCat(WCHAR* destination, const WCHAR* source);
    virtual std::wstring ConvertDateToString(SYSTEMTIME& DateTime);
    virtual std::wstring ConvertTimeToString(SYSTEMTIME& DateTime);
    virtual std::wstring ConvertIntToString(DWORD64 num);
    virtual std::wstring GetEndLineChars();

};

