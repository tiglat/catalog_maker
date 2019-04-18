#pragma once

#include "windows.h"

#include "IStringOperations.h"

class WideStringOperations : public IStringOperations<WCHAR>
{
public:
    virtual bool IsNotNullOrEmpty(const WCHAR* pStr);
    virtual size_t StrLen(const WCHAR* pStr);
    virtual WCHAR* StrRChr(WCHAR* pStr, const WCHAR ch);
    virtual UINT StrNChr(const WCHAR* pStr, const WCHAR ch);
    virtual INT StrCaseInsensitiveCmp(const WCHAR* pLhsStr, const WCHAR* pRhsStr);
};

