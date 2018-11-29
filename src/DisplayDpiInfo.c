//
// DisplayDpiInfo.c
//
// Fills the DPI tab-pane with info for the current window.
//

#include "WinSpy.h"

#include "resource.h"


//
// Definitions from the platform SDK
//
#ifndef DPI_AWARENESS_CONTEXT_UNAWARE

DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);

#define DPI_AWARENESS_CONTEXT_UNAWARE               ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE          ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED     ((DPI_AWARENESS_CONTEXT)-5)

#endif



//
// These APIs exist on Windows 10 and later only.
//
typedef UINT (WINAPI * PFN_GetDpiForWindow)(HWND);
typedef DPI_AWARENESS_CONTEXT (WINAPI * PFN_GetWindowDpiAwarenessContext)(HWND);
typedef BOOL (WINAPI * PFN_AreDpiAwarenessContextsEqual)(DPI_AWARENESS_CONTEXT, DPI_AWARENESS_CONTEXT);
typedef BOOL (WINAPI * PFN_GetProcessDpiAwareness)(HANDLE, int *);
typedef DPI_AWARENESS_CONTEXT (WINAPI * PFN_GetDpiAwarenessContextForProcess)(HANDLE);
typedef UINT (WINAPI * PFN_GetDpiFromDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);

static PFN_GetDpiForWindow s_pfnGetDpiForWindow = NULL;
static PFN_GetWindowDpiAwarenessContext s_pfnGetWindowDpiAwarenessContext = NULL;
static PFN_AreDpiAwarenessContextsEqual s_pfnAreDpiAwarenessContextsEqual = NULL;
static PFN_GetProcessDpiAwareness s_pfnGetProcessDpiAwareness = NULL;
static PFN_GetDpiAwarenessContextForProcess s_pfnGetDpiAwarenessContextForProcess = NULL;
static PFN_GetDpiFromDpiAwarenessContext s_pfnGetDpiFromDpiAwarenessContext = NULL;

static BOOL s_fCheckedForAPIs = FALSE;

void InitializeDpiApis()
{
    if (!s_fCheckedForAPIs)
    {
        HMODULE hmod = GetModuleHandle(L"user32");

        s_pfnGetDpiForWindow = (PFN_GetDpiForWindow)GetProcAddress(hmod, "GetDpiForWindow");
        s_pfnGetWindowDpiAwarenessContext = (PFN_GetWindowDpiAwarenessContext)GetProcAddress(hmod, "GetWindowDpiAwarenessContext");
        s_pfnAreDpiAwarenessContextsEqual = (PFN_AreDpiAwarenessContextsEqual)GetProcAddress(hmod, "AreDpiAwarenessContextsEqual");
        s_pfnGetProcessDpiAwareness = (PFN_GetProcessDpiAwareness)GetProcAddress(hmod, "GetProcessDpiAwareness");
        s_pfnGetDpiAwarenessContextForProcess = (PFN_GetDpiAwarenessContextForProcess)GetProcAddress(hmod, "GetDpiAwarenessContextForProcess");
        s_pfnGetDpiFromDpiAwarenessContext = (PFN_GetDpiFromDpiAwarenessContext)GetProcAddress(hmod, "GetDpiFromDpiAwarenessContext");

        s_fCheckedForAPIs = TRUE;
    }
}

void DescribeDpiAwarenessContext(PSTR pszBuffer, size_t cchBuffer, DPI_AWARENESS_CONTEXT dpiContext, BOOL fIncludeDpiValue)
{
    PSTR pszValue = NULL;

    if (s_pfnAreDpiAwarenessContextsEqual(dpiContext, DPI_AWARENESS_CONTEXT_UNAWARE))
    {
        pszValue = "Unaware";
    }
    else if (s_pfnAreDpiAwarenessContextsEqual(dpiContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
    {
        pszValue = "Per-Monitor Aware";
    }
    else if (s_pfnAreDpiAwarenessContextsEqual(dpiContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    {
        pszValue = "Per-Monitor Aware v2";
    }
    else if (s_pfnAreDpiAwarenessContextsEqual(dpiContext, DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED))
    {
        pszValue = "Unaware (GDI Scaled)";
    }
    else if (s_pfnAreDpiAwarenessContextsEqual(dpiContext, DPI_AWARENESS_CONTEXT_SYSTEM_AWARE))
    {
        if (fIncludeDpiValue && s_pfnGetDpiFromDpiAwarenessContext)
        {
            UINT dpi = s_pfnGetDpiFromDpiAwarenessContext(dpiContext);
            sprintf_s(pszBuffer, cchBuffer, "System Aware  (%u DPI)", dpi);
        }
        else
        {
            pszValue = "System Aware";
        }
    }
    else
    {
        sprintf_s(pszBuffer, cchBuffer, "Unknown (0x%08p)", dpiContext);
    }

    if (pszValue)
    {
        StringCchCopyA(pszBuffer, cchBuffer, pszValue);
    }
}

void DescribeProcessDpiAwareness(PSTR pszBuffer, size_t cchBuffer, DWORD dwProcessId)
{
    CHAR  szValue[MAX_PATH] = "?";
    PCSTR pszValue = szValue;
    HANDLE hProcess = NULL;

    InitializeDpiApis();

    if (s_pfnGetDpiAwarenessContextForProcess || s_pfnGetProcessDpiAwareness)
    {
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);

        if (!hProcess)
        {
            DWORD dwError = GetLastError();

            if (dwError == ERROR_ACCESS_DENIED)
            {
                pszValue = "<Access Denied>";
            }

        }
    }
    else
    {
        pszValue = "<Unavailable>";
    }

    if (hProcess)
    {
        if (s_pfnGetDpiAwarenessContextForProcess)
        {
            DPI_AWARENESS_CONTEXT dpiContext = s_pfnGetDpiAwarenessContextForProcess(hProcess);

            if (dpiContext)
            {
                DescribeDpiAwarenessContext(szValue, ARRAYSIZE(szValue), dpiContext, TRUE);
            }
        }
        else if (s_pfnGetProcessDpiAwareness)
        {
            int dwLevel;

            if (s_pfnGetProcessDpiAwareness(hProcess, &dwLevel))
            {
                switch (dwLevel)
                {
                    case 0: // PROCESS_DPI_UNAWARE
                        pszValue = "Unaware";
                        break;

                    case 1: // PROCESS_SYSTEM_DPI_AWARE
                        pszValue = "System Aware";
                        break;

                    case 2: // PROCESS_PER_MONITOR_DPI_AWARE
                        pszValue = "Per-Monitor Aware";
                        break;

                    default:
                        sprintf_s(szValue, ARRAYSIZE(szValue), "Unknown (%d)", dwLevel);
                        break;
                }
            }
        }
    }

    if (hProcess)
    {
        CloseHandle(hProcess);
    }

    StringCchCopyA(pszBuffer, cchBuffer, pszValue);
}

//
// Update the DPI tab for the specified window
//
void SetDpiInfo(HWND hwnd)
{
    HWND hwndDlg = WinSpyTab[DPI_TAB].hwnd;
    CHAR szTemp[100];
    PSTR pszValue = NULL;
    BOOL fValid;

    InitializeDpiApis();

    fValid = (hwnd && IsWindow(hwnd));

    if (!fValid)
    {
        pszValue = (hwnd == NULL) ? "" : "(invalid window)";
    }

    // DPI field

    if (fValid)
    {
        if (s_pfnGetDpiForWindow)
        {
            UINT dpi     = s_pfnGetDpiForWindow(hwnd);
            UINT percent = (UINT)(dpi * 100 / 96);

            sprintf_s(szTemp, ARRAYSIZE(szTemp), "%d (%u%%)", dpi, percent);
            pszValue = szTemp;
        }
        else
        {
            pszValue = "<Unavailable>";
        }
    }

    SetDlgItemTextA(hwndDlg, IDC_WINDOW_DPI, pszValue);

    // DPI awareness field

    if (fValid)
    {
        if (s_pfnGetWindowDpiAwarenessContext)
        {
            DPI_AWARENESS_CONTEXT dpiContext = s_pfnGetWindowDpiAwarenessContext(hwnd);

            DescribeDpiAwarenessContext(szTemp, ARRAYSIZE(szTemp), dpiContext, FALSE);
            pszValue = szTemp;
        }
        else
        {
            pszValue = "<Unavailable>";
        }
    }

    SetDlgItemTextA(hwndDlg, IDC_WINDOW_DPI_AWARENESS, pszValue);
}

