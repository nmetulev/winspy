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

int WINAPI GetRectHeight(RECT *rect);
int WINAPI GetRectWidth(RECT *rect);

DWORD_PTR GetDlgItemBaseInt(HWND hwnd, UINT ctrlid, int base);
DWORD_PTR _tstrtoib16(PCTSTR pszHexStr);
BOOL EnableDialogTheme(HWND hwnd);

BOOL EnableDebugPrivilege();

TCHAR *GetVersionString(TCHAR *szFileName, TCHAR *szValue, TCHAR *szBuffer, ULONG nLength);

BOOL ProcessArchMatches(HWND hwnd);
WORD GetProcessorArchitecture();

HWND GetRealParent(HWND hWnd);

BOOL CopyTextToClipboard(HWND hWnd, TCHAR *psz);

HBITMAP LoadPNGImage(UINT id, void **bits);

HBITMAP ExpandNineGridImage(SIZE outputSize, HBITMAP hbmSrc, RECT edges);

void UpdateLayeredWindowContent(HWND hwnd, RECT rc, HBITMAP hbmp, BYTE alpha);

BOOL IsWindowsFormsClassName(PCWSTR pcszClass);
void ExtractWindowsFormsInnerClassName(PWSTR pszName, size_t cchName);

#ifdef __cplusplus
}
#endif

#endif