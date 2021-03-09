
#include <string>
#include "WideStringOperations.h"

using namespace std;

const WCHAR* IStringOperations<WCHAR, wstring>::FILE_NAME_COLUMN   = L"File name";
const WCHAR* IStringOperations<WCHAR, wstring>::EXT_COLUMN         = L"Ext";
const WCHAR* IStringOperations<WCHAR, wstring>::SIZE_COLUMN        = L"Size";
const WCHAR* IStringOperations<WCHAR, wstring>::DATE_COLUMN        = L"Date";
const WCHAR* IStringOperations<WCHAR, wstring>::TIME_COLUMN        = L"Time";
const WCHAR* IStringOperations<WCHAR, wstring>::ATTR_COLUMN        = L"Attr";

const WCHAR* IStringOperations<WCHAR, wstring>::FOOTER_TOTAL_FILES = L"\r\ntotal files ";
const WCHAR* IStringOperations<WCHAR, wstring>::FOOTER_TOTAL_SIZE  = L"    total size ";

bool WideStringOperations::IsNotNullOrEmpty(const WCHAR* pStr)
{
    return pStr != nullptr && wcslen(pStr) > 0;
}

size_t WideStringOperations::StrLen(const WCHAR* pStr)
{
    return wcslen(pStr);
}

WCHAR* WideStringOperations::StrChr(WCHAR* pStr, WCHAR chr)
{
    return wcschr(pStr, chr);
}

WCHAR* WideStringOperations::StrRChr(WCHAR* pStr, WCHAR chr)
{
    return wcsrchr(pStr, chr);
}

UINT WideStringOperations::StrNChr(const WCHAR* pStr, const WCHAR ch)
{
    UINT len = (UINT)wcslen(pStr);
    UINT Counter = 0;

    for (UINT i = 0; i < len; i++)
    {
        if (pStr[i] == ch)
            Counter++;
    }

    return (Counter);
}


INT WideStringOperations::StrCaseInsensitiveCmp(const WCHAR* pLhsStr, const WCHAR* pRhsStr)
{
    return _wcsicmp(pLhsStr, pRhsStr);
}

std::wstring WideStringOperations::ConvertDateToString(SYSTEMTIME& DateTime)
{
    const USHORT BufLen = 13;
    char buf[BufLen];

    snprintf(buf, BufLen, "%02d.%02d.%4d  ", DateTime.wDay, DateTime.wMonth, DateTime.wYear);

    WCHAR wbuf[BufLen];
    USHORT WBufLen = sizeof(wbuf);
    memset(wbuf, 0, WBufLen);
    MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, WBufLen);

    std::wstring rv(wbuf);

    return rv;
}


std::wstring WideStringOperations::ConvertTimeToString(SYSTEMTIME& DateTime)
{
    const USHORT BufLen =11;
    char buf[BufLen];

    snprintf(buf, BufLen, "%02d:%02d.%02d  ", DateTime.wHour, DateTime.wMinute, DateTime.wSecond);

    WCHAR wbuf[BufLen];
    USHORT WBufLen = sizeof(wbuf);
    memset(wbuf, 0, WBufLen);
    MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, WBufLen);

    std::wstring rv(wbuf);

    return rv;
}


wstring WideStringOperations::ConvertFileSizeToString(DWORD64 num)
{
    wstring source = to_wstring(num);

    if (source.length() <= 3)
    {
        return source;
    }

    auto remainderPart = source.length() % 3;

    wstring result;
    int j = 0;

    for (size_t i = 0; i < source.length(); i++)
    {
        result.push_back(source[i]);
        j++;

        if (i + 1 == remainderPart)
        {
            result.push_back(',');
            j = 0;
        }


        if (j == 3 && i < source.length() - 1)
        {
            result.push_back(',');
            j = 0;
        }
    }

    return result;
}

wstring WideStringOperations::ConvertIntToString(DWORD num)
{
    return std::to_wstring(num);
}

DWORD64 WideStringOperations::ConvertStringToInt(std::wstring& Str)
{
    return _wtoi64(Str.c_str());
}

DWORD64 WideStringOperations::ConvertStringToInt(const WCHAR* Str)
{
    return _wtoi64(Str);
}

wstring WideStringOperations::GetEndLineChars()
{
    return L"\r\n";
}


WCHAR* WideStringOperations::StrTok(WCHAR* strToken, const WCHAR* strDelimit)
{
    return wcstok(strToken, strDelimit);
}

WCHAR* WideStringOperations::StrCpy(WCHAR* destination, const WCHAR* source)
{
    return wcscpy(destination, source);
}

WCHAR* WideStringOperations::StrCat(WCHAR* destination, const WCHAR* source)
{
    return wcscat(destination, source);
}

WCHAR* WideStringOperations::StrNCpy(WCHAR* destination, const WCHAR* source, UINT len)
{
    return wcsncpy(destination, source, len);
}

WCHAR* WideStringOperations::StrNCat(WCHAR* destination, const WCHAR* source, UINT len)
{
    return wcsncat(destination, source, len);
}

WCHAR* WideStringOperations::StrStr(const WCHAR* destination, const WCHAR* source)
{
    return const_cast<WCHAR*> (wcsstr(destination, source));
}
