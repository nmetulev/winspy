//
//  RegHelper.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Implements registry helper functions
//

#include "WinSpy.h"
#include "RegHelper.h"


LONG GetSettingBinary(HKEY hkey, PCWSTR szKeyName, void *buf, ULONG nNumBytes)
{
    DWORD type = REG_BINARY;

    if (ERROR_SUCCESS == RegQueryValueEx(hkey, szKeyName, 0, &type, (BYTE *)buf, &nNumBytes))
    {
        if (type != REG_BINARY)
            return 0;

        return nNumBytes;
    }
    else
    {
        return 0;
    }
}

LONG GetSettingInt(HKEY hkey, PCWSTR szKeyName, LONG nDefault)
{
    DWORD type;
    LONG value;
    ULONG len = sizeof(value);

    if (ERROR_SUCCESS == RegQueryValueEx(hkey, szKeyName, 0, &type, (BYTE *)&value, &len))
    {
        if (type != REG_DWORD)
            return nDefault;

        return value;
    }
    else
    {
        return nDefault;
    }
}

BOOL GetSettingBool(HKEY hkey, PCWSTR szKeyName, BOOL nDefault)
{
    DWORD type;
    BOOL  value;
    ULONG len = sizeof(value);

    if (ERROR_SUCCESS == RegQueryValueEx(hkey, szKeyName, 0, &type, (BYTE *)&value, &len))
    {
        if (type != REG_DWORD)
            return nDefault;

        return value != 0;
    }
    else
    {
        return nDefault;
    }
}

LONG GetSettingStr(HKEY hkey, PCWSTR szKeyName, PCWSTR szDefault, PWSTR szReturnStr, DWORD nSize)
{
    DWORD type = REG_SZ;
    PCWSTR bigbuf[256];
    ULONG len = sizeof(bigbuf);

    if (ERROR_SUCCESS == RegQueryValueEx(hkey, szKeyName, 0, &type, (BYTE *)bigbuf, &len))
    {
        if (type != REG_SZ)
            return 0;

        memcpy(szReturnStr, bigbuf, len + sizeof(WCHAR));
        return len;
    }
    else
    {
        len = min(nSize, (DWORD)wcslen(szDefault) * sizeof(WCHAR));
        memcpy(szReturnStr, szDefault, len + sizeof(WCHAR));
        return len;
    }
}

LONG WriteSettingInt(HKEY hkey, PCWSTR szKeyName, LONG nValue)
{
    return RegSetValueEx(hkey, szKeyName, 0, REG_DWORD, (BYTE *)&nValue, sizeof(nValue));
}

LONG WriteSettingBool(HKEY hkey, PCWSTR szKeyName, BOOL nValue)
{
    return RegSetValueEx(hkey, szKeyName, 0, REG_DWORD, (BYTE *)&nValue, sizeof(nValue));
}

LONG WriteSettingStr(HKEY hkey, PCWSTR szKeyName, PCWSTR szString)
{
    return RegSetValueEx(hkey, szKeyName, 0, REG_SZ, (BYTE *)szString, (DWORD)(wcslen(szString) + 1) * sizeof(WCHAR));
}

LONG WriteSettingBinary(HKEY hkey, PCWSTR szKeyName, void *buf, UINT nNumBytes)
{
    return RegSetValueEx(hkey, szKeyName, 0, REG_BINARY, (BYTE *)buf, nNumBytes);
}
