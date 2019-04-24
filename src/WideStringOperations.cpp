
#include <string>
#include "WideStringOperations.h"

using namespace std;

bool WideStringOperations::IsNotNullOrEmpty(const WCHAR* pStr)
{
    return pStr != nullptr && wcslen(pStr) > 0;
}

size_t WideStringOperations::StrLen(const WCHAR* pStr)
{
    return wcslen(pStr);
}


WCHAR* WideStringOperations::StrRChr(WCHAR* pStr, WCHAR chr)
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
    MultiByteToWideChar(CP_ACP, 0, buf, BufLen, wbuf, WBufLen);

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
    MultiByteToWideChar(CP_ACP, 0, buf, BufLen, wbuf, WBufLen);

    std::wstring rv(wbuf);

    return rv;
}


wstring WideStringOperations::ConvertIntToString(DWORD64 num)
{
    wstring source = to_wstring(num);

    if (source.length() <= 3)
    {
        return source;
    }

    auto remainderPart = source.length() % 3;

    wstring result;
    int j = 0;

    for (auto i = 0; i < source.length(); i++)
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

wstring WideStringOperations::GetEndLineChars()
{
    return L"\r\n";
}