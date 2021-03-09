#include "AnsiStringOperations.h"

using namespace std;

const char* IStringOperations<char, string>::FILE_NAME_COLUMN   = "File name";
const char* IStringOperations<char, string>::EXT_COLUMN         = "Ext";
const char* IStringOperations<char, string>::SIZE_COLUMN        = "Size";
const char* IStringOperations<char, string>::DATE_COLUMN        = "Date";
const char* IStringOperations<char, string>::TIME_COLUMN        = "Time";
const char* IStringOperations<char, string>::ATTR_COLUMN        = "Attr";

const char* IStringOperations<char, string>::FOOTER_TOTAL_FILES = "\r\ntotal files ";
const char* IStringOperations<char, string>::FOOTER_TOTAL_SIZE  = "    total size ";


bool AnsiStringOperations::IsNotNullOrEmpty(const char* pStr)
{
    return pStr != nullptr && strlen(pStr) > 0;
}

size_t AnsiStringOperations::StrLen(const char* pStr)
{
    return strlen(pStr);
}


char* AnsiStringOperations::StrChr(char* pStr, char chr)
{
    return strchr(pStr, chr);
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

    for (UINT i = 0; i < len; i++)
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

string AnsiStringOperations::ConvertFileSizeToString(DWORD64 num)
{
    string source = to_string(num);

    if (source.length() <= 3)
    {
        return source;
    }

    auto remainderPart = source.length() % 3;

    string result;
    size_t j = 0;

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

string AnsiStringOperations::ConvertIntToString(DWORD num)
{
    return std::to_string(num);
}

DWORD64 AnsiStringOperations::ConvertStringToInt(std::string& Str)
{
    return _atoi64(Str.c_str());
}

DWORD64 AnsiStringOperations::ConvertStringToInt(const char* Str)
{
    return _atoi64(Str);
}

string AnsiStringOperations::GetEndLineChars()
{
    return "\r\n";
}

char* AnsiStringOperations::StrTok(char* strToken, const char* strDelimit)
{
    return strtok(strToken, strDelimit);
}

char* AnsiStringOperations::StrCpy(char* destination, const char* source)
{
    return strcpy(destination, source);
}

char* AnsiStringOperations::StrNCpy(char* destination, const char* source, UINT len)
{
    return strncpy(destination, source, len);
}


char* AnsiStringOperations::StrCat(char* destination, const char* source)
{
    return strcat(destination, source);
}

char* AnsiStringOperations::StrNCat(char* destination, const char* source, UINT len)
{
    return strncat(destination, source, len);
}

char* AnsiStringOperations::StrStr(const char* destination, const char* source)
{
    return const_cast<char*> (strstr(destination, source));
}

