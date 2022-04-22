#ifndef _REGHELPER_INCLUDED
#define _REGHELPER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

LONG GetSettingInt(HKEY hkey, PCWSTR szKeyName, LONG nDefault);
BOOL GetSettingBool(HKEY hkey, PCWSTR szKeyName, BOOL nDefault);
LONG GetSettingStr(HKEY hkey, PCWSTR szKeyName, PCWSTR szDefault, PWSTR szReturnStr, DWORD nSize);
LONG GetSettingBinary(HKEY hkey, PCWSTR szKeyName, void *buf, ULONG nNumBytes);

LONG WriteSettingInt(HKEY hkey, PCWSTR szKeyName, LONG nValue);
LONG WriteSettingBool(HKEY hkey, PCWSTR szKeyName, BOOL nValue);
LONG WriteSettingStr(HKEY hkey, PCWSTR szKeyName, PCWSTR szString);
LONG WriteSettingBinary(HKEY hkey, PCWSTR szKeyName, void *buf, UINT nNumBytes);

#ifdef __cplusplus
}
#endif

#endif