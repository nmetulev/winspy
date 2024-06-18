#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

BOOL StrBeginsWith(PCWSTR pcsz, PCWSTR pcszPrefix);

UINT AddStyle(HWND hwnd, UINT style);
UINT AddDlgItemStyle(HWND hwnd, UINT nCtrlId, UINT style);
UINT DelStyle(HWND hwnd, UINT style);
UINT DelDlgItemStyle(HWND hwnd, UINT nCtrlId, UINT style);
BOOL EnableDlgItem(HWND hwnd, UINT nCtrlId, BOOL fEnabled);
BOOL ShowDlgItem(HWND hwnd, UINT nCtrlId, DWORD dwShowCmd);
void SetDlgItemTextEx(HWND hwndDlg, UINT nCtrlId, PCWSTR pcsz);
void SetDlgItemTextExA(HWND hwndDlg, UINT nCtrlId, PCSTR pcsz);

void FormatDlgItemText(HWND hwndDlg, UINT id, _Printf_format_string_ PCWSTR pcszFormat, ...);

int WINAPI GetRectHeight(RECT *rect);
int WINAPI GetRectWidth(RECT *rect);

DWORD_PTR GetDlgItemBaseInt(HWND hwnd, UINT ctrlid, int base);
DWORD_PTR _tstrtoib16(PCWSTR pszHexStr);
BOOL EnableDialogTheme(HWND hwnd);

BOOL EnableDebugPrivilege();

WCHAR *GetVersionString(WCHAR *szFileName, WCHAR *szValue, WCHAR *szBuffer, ULONG nLength);

BOOL ProcessArchMatches(HWND hwnd);
WORD GetProcessorArchitecture();

HWND GetRealParent(HWND hWnd);

BOOL CopyTextToClipboard(HWND hWnd, WCHAR *psz);

HBITMAP LoadPNGImage(UINT id, void **bits);

HBITMAP ExpandNineGridImage(SIZE outputSize, HBITMAP hbmSrc, RECT edges);

void UpdateLayeredWindowContent(HWND hwnd, RECT rc, HBITMAP hbmp, BYTE alpha);

BOOL IsWindowsFormsClassName(PCWSTR pcszClass);
void ExtractWindowsFormsInnerClassName(PWSTR pszName);

RECT GetControlRect(HWND hwndParent, HWND hwnd);
void SetControlRect(HWND hwnd, RECT* prc);

#ifdef __cplusplus
}
#endif

#endif
