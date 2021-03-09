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
    virtual WCHAR* StrStr(const WCHAR* destination, const WCHAR* source);

    virtual WCHAR* StrNCpy(WCHAR* destination, const WCHAR* source, UINT len);
    virtual WCHAR* StrNCat(WCHAR* destination, const WCHAR* source, UINT len);

    virtual std::wstring ConvertDateToString(SYSTEMTIME& DateTime);
    virtual std::wstring ConvertTimeToString(SYSTEMTIME& DateTime);
    virtual std::wstring ConvertFileSizeToString(DWORD64 num);
    virtual std::wstring ConvertIntToString(DWORD num);
    virtual DWORD64 ConvertStringToInt(std::wstring& Str);
    virtual DWORD64 ConvertStringToInt(const WCHAR* Str);

    virtual std::wstring GetEndLineChars();

};

