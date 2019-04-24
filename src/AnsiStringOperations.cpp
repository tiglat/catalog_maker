#include "AnsiStringOperations.h"

using namespace std;

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

/*****************************************************************************
    Routine:     StrNChr
------------------------------------------------------------------------------
    Description:
                Count the number of "ch" symbols in the full file name

    Arguments:
                pFileName	- full file name

    Return Value:
                Return number of "ch" symbols

*****************************************************************************/

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


string AnsiStringOperations::ConvertDateToString(SYSTEMTIME& DateTime)
{
    const USHORT BufLen = 13;
    char buf[BufLen];

    snprintf(buf, BufLen, "%02d.%02d.%4d  ", DateTime.wDay, DateTime.wMonth, DateTime.wYear);
    string rv(buf);

    return rv;
}


string AnsiStringOperations::ConvertTimeToString(SYSTEMTIME& DateTime)
{
    const USHORT BufLen = 11;
    char buf[BufLen];

    snprintf(buf, BufLen, "%02d:%02d.%02d  ", DateTime.wHour, DateTime.wMinute, DateTime.wSecond);
    string rv(buf);

    return rv;
}


/*****************************************************************************
    Routine:     FormatIntNumber
------------------------------------------------------------------------------
    Description:
                Convert num to string and insert digit
                group separator ',' between thousands

    Arguments:
                num	- number for converting

    Return Value:
                formated string

*****************************************************************************/

string AnsiStringOperations::ConvertIntToString(DWORD64 num)
{
    string source = to_string(num);

    if (source.length() <= 3)
    {
        return source;
    }

    auto remainderPart = source.length() % 3;

    string result;
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

string AnsiStringOperations::GetEndLineChars()
{
    return "\r\n";
}