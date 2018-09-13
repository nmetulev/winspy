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

static PFN_GetDpiForWindow s_pfnGetDpiForWindow = NULL;
static PFN_GetWindowDpiAwarenessContext s_pfnGetWindowDpiAwarenessContext = NULL;
static PFN_AreDpiAwarenessContextsEqual s_pfnAreDpiAwarenessContextsEqual = NULL;
static BOOL s_fCheckedForAPIs = FALSE;

void InitializeDpiApis()
{
    if (!s_fCheckedForAPIs)
    {
        HMODULE hmod = GetModuleHandle(L"user32");

        s_pfnGetDpiForWindow = (PFN_GetDpiForWindow)GetProcAddress(hmod, "GetDpiForWindow");
        s_pfnGetWindowDpiAwarenessContext = (PFN_GetWindowDpiAwarenessContext)GetProcAddress(hmod, "GetWindowDpiAwarenessContext");
        s_pfnAreDpiAwarenessContextsEqual = (PFN_AreDpiAwarenessContextsEqual)GetProcAddress(hmod, "AreDpiAwarenessContextsEqual");

        s_fCheckedForAPIs = TRUE;
    }
}


void DescribeDpiAwarenessContext(PSTR pszBuffer, size_t cchBuffer, DPI_AWARENESS_CONTEXT dpiContext)
{
    PSTR pszValue = NULL;

    if (s_pfnAreDpiAwarenessContextsEqual)
    {
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
            pszValue = "System Aware";
        }
    }

    if (pszValue)
    {
        StringCchCopyA(pszBuffer, cchBuffer, pszValue);
    }
    else
    {
        sprintf_s(pszBuffer, cchBuffer, "Unknown (0x%08p)", dpiContext);
    }
}


//
// Update the DPI tab for the specified window
//
void SetDpiInfo(HWND hwnd)
{
    HWND hwndDlg = WinSpyTab[DPI_TAB].hwnd;
    CHAR szTemp[100];
    PSTR pszValue;

    InitializeDpiApis();

    // DPI field

    if (s_pfnGetDpiForWindow)
    {
        UINT dpi     = s_pfnGetDpiForWindow(hwnd);
        UINT percent = (UINT)(dpi * 100 / 96);

        sprintf_s(szTemp, ARRAYSIZE(szTemp), "%d (%u%%)", dpi, percent);
        pszValue = szTemp;
    }
    else
    {
        pszValue = "<UNAVAILABLE>";
    }

    SetDlgItemTextA(hwndDlg, IDC_WINDOW_DPI, pszValue);

    // DPI awareness field

    if (s_pfnGetWindowDpiAwarenessContext)
    {
        DPI_AWARENESS_CONTEXT dpiContext = s_pfnGetWindowDpiAwarenessContext(hwnd);

        DescribeDpiAwarenessContext(szTemp, ARRAYSIZE(szTemp), dpiContext);
        pszValue = szTemp;
    }
    else
    {
        pszValue = "<UNAVAILABLE>";
    }

    SetDlgItemTextA(hwndDlg, IDC_WINDOW_DPI_AWARENESS, pszValue);
}

