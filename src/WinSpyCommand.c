//
//  WinSpyCommand.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Menu / Control Command handler
//

#include "WinSpy.h"

#include "resource.h"
#include "Utils.h"
#include "FindTool.h"
#include "CaptureWindow.h"

void SetPinState(BOOL fPinned)
{
    g_opts.fPinWindow = fPinned;

    SendMessage(hwndPin, TB_CHANGEBITMAP, IDM_WINSPY_PIN,
        MAKELPARAM(fPinned, 0));

    SendMessage(hwndPin, TB_CHECKBUTTON, IDM_WINSPY_PIN,
        MAKELPARAM(fPinned, 0));

}

UINT WinSpyDlg_CommandHandler(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    NMHDR       hdr;
    UINT        uLayout;

    HWND hwndGeneral;
    HWND hwndFocus;
    HWND hwndCtrl;

    switch (LOWORD(wParam))
    {
    case IDM_WINSPY_ONTOP:

        if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
        {
            // Not top-most any more
            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_ZONLY);
        }
        else
        {
            // Make top-most (float above all other windows)
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ZONLY);
        }

        return TRUE;

    case IDM_WINSPY_TOGGLE:
        ToggleWindowLayout(hwnd);
        return TRUE;

    case IDM_WINSPY_TOGGLEEXP:

        uLayout = GetWindowLayout(hwnd);

        if (uLayout == WINSPY_EXPANDED)
            SetWindowLayout(hwnd, WINSPY_MINIMIZED);

        else if (uLayout == WINSPY_NORMAL)
            SetWindowLayout(hwnd, WINSPY_EXPANDED);

        else if (uLayout == WINSPY_MINIMIZED)
            SetWindowLayout(hwnd, WINSPY_NORMAL);

        return TRUE;

    case IDM_WINSPY_ZOOMTL:
        WinSpy_ZoomTo(hwnd, PINNED_TOPLEFT);
        return TRUE;

    case IDM_WINSPY_ZOOMTR:
        WinSpy_ZoomTo(hwnd, PINNED_TOPRIGHT);
        return TRUE;

    case IDM_WINSPY_ZOOMBR:
        WinSpy_ZoomTo(hwnd, PINNED_BOTTOMRIGHT);
        return TRUE;

    case IDM_WINSPY_ZOOMBL:
        WinSpy_ZoomTo(hwnd, PINNED_BOTTOMLEFT);
        return TRUE;

    case IDM_WINSPY_REFRESH:
        DisplayWindowInfo(spy_hCurWnd);
        return TRUE;

    case IDM_WINSPY_OPTIONS:
        ShowOptionsDlg(hwnd);
        return TRUE;

    case IDM_WINSPY_PIN:

        g_opts.fPinWindow = !g_opts.fPinWindow;

        SendMessage(hwndPin, TB_CHANGEBITMAP, IDM_WINSPY_PIN,
            MAKELPARAM(g_opts.fPinWindow, 0));

        // if from an accelerator, then we have to manually check the
        if (HIWORD(wParam) == 1)
        {
            SendMessage(hwndPin, TB_CHECKBUTTON, IDM_WINSPY_PIN,
                MAKELPARAM(g_opts.fPinWindow, 0));
        }

        GetPinnedPosition(hwnd, &g_opts.ptPinPos);
        return TRUE;

    case IDC_HIDDEN:
        g_opts.fShowHidden = IsDlgButtonChecked(hwnd, IDC_HIDDEN);
        return TRUE;

    case IDC_MINIMIZE:
        g_opts.fMinimizeWinSpy = IsDlgButtonChecked(hwnd, IDC_MINIMIZE);
        return TRUE;

    case IDM_WINSPY_GENERAL:
    case IDM_WINSPY_STYLES:
    case IDM_WINSPY_PROPERTIES:
    case IDM_WINSPY_CLASS:
    case IDM_WINSPY_WINDOWS:
    case IDM_WINSPY_SCROLLBARS:

        // Simulate the tab-control being clicked
        hdr.hwndFrom = GetDlgItem(hwnd, IDC_TAB1);
        hdr.idFrom = IDC_TAB1;
        hdr.code = TCN_SELCHANGE;

        TabCtrl_SetCurSel(hdr.hwndFrom, LOWORD(wParam) - IDM_WINSPY_GENERAL);

        SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)&hdr);

        return TRUE;

    case IDC_FLASH:

        HWND hwndSelected = WindowTree_GetSelectedWindow();

        if (hwndSelected)
        {
            FlashWindowBorder(hwndSelected);
        }

        return TRUE;

    case IDC_EXPAND:

        if (GetWindowLayout(hwnd) == WINSPY_NORMAL)
        {
            WindowTree_Refresh(spy_hCurWnd, FALSE);
            SetWindowLayout(hwnd, WINSPY_EXPANDED);
        }
        else
        {
            SetWindowLayout(hwnd, WINSPY_NORMAL);
        }

        return TRUE;

    case IDC_CAPTURE:
        CaptureWindow(hwnd, spy_hCurWnd);
        MessageBox(hwnd, L"Window contents captured to clipboard", szAppName, MB_ICONINFORMATION);
        return TRUE;

    case IDC_AUTOUPDATE:
        if (IsDlgButtonChecked(hwnd, IDC_AUTOUPDATE))
            SetTimer(hwnd, 0, 1000, NULL);
        else
            KillTimer(hwnd, 0);
        return TRUE;

    case IDOK:

        hwndGeneral = WinSpyTab[GENERAL_TAB].hwnd;
        hwndFocus = GetFocus();

        if (hwndFocus == GetDlgItem(hwndGeneral, IDC_HANDLE))
        {
            hwndCtrl = (HWND)GetDlgItemBaseInt(hwndGeneral, IDC_HANDLE, 16);

            if (IsWindow(hwndCtrl))
            {
                spy_hCurWnd = hwndCtrl;
                DisplayWindowInfo(spy_hCurWnd);
            }

            return 0;
        }
        else if (hwndFocus == GetDlgItem(hwndGeneral, IDC_CAPTION1) ||
            hwndFocus == GetWindow(GetDlgItem(hwndGeneral, IDC_CAPTION2), GW_CHILD))
        {
            PostMessage(hwndGeneral, WM_COMMAND, MAKEWPARAM(IDC_SETCAPTION, BN_CLICKED), 0);
            return FALSE;
        }


        if (GetFocus() != (HWND)lParam)
            return FALSE;

        ExitWinSpy(hwnd, 0);

        return TRUE;

    case IDC_LOCATE:

        if (spy_hCurWnd)
        {
            WindowTree_Locate(spy_hCurWnd);
        }

        return TRUE;

    case IDC_REFRESH:

        WindowTree_Refresh(spy_hCurWnd, TRUE);

        return TRUE;
    }

    return FALSE;
}

UINT WinSpyDlg_SysMenuHandler(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    WCHAR szText[200];
    WCHAR szTitle[60];
    WCHAR szVersion[40];
    WCHAR szCurExe[MAX_PATH];

    switch (wParam & 0xFFF0)
    {
    case SC_RESTORE:

        if (IsWindowMinimized(hwnd))
        {
            break;
        }
        else
        {
            ToggleWindowLayout(hwnd);
            return 1;
        }

    case SC_MAXIMIZE:
        ToggleWindowLayout(hwnd);
        return 1;
    }

    switch (wParam)
    {
    case IDM_WINSPY_ABOUT:

        GetModuleFileName(0, szCurExe, MAX_PATH);
        GetVersionString(szCurExe, TEXT("FileVersion"), szVersion, 40);

        swprintf_s(szText, ARRAYSIZE(szText),
            L"%s v%s\n"
            L"\n"
            L"Copyright(c) 2002-2012 by Catch22 Productions.\n"
            L"Written by J Brown.\n"
            L"Homepage: www.catch22.net\n"
            L"\n"
            L"Forked and improved by RaMMicHaeL.\n"
            L"Homepage: rammichael.com",
            szAppName, szVersion);
        swprintf_s(szTitle, ARRAYSIZE(szTitle), L"About %s", szAppName);

        MessageBox(hwnd, szText, szTitle, MB_OK | MB_ICONINFORMATION);
        return TRUE;

    case IDM_WINSPY_OPTIONS:
        ShowOptionsDlg(hwnd);
        return TRUE;

    case IDM_WINSPY_BROADCASTER:
        ShowBroadcasterDlg(hwnd);
        return TRUE;

    case IDM_WINSPY_ONTOP:

        PostMessage(hwnd, WM_COMMAND, wParam, lParam);
        return TRUE;

    }
    return FALSE;

}

UINT WinSpyDlg_TimerHandler(UINT_PTR uTimerId)
{
    if (uTimerId == 0)
    {
        DisplayWindowInfo(spy_hCurWnd);
        return TRUE;
    }

    return FALSE;
}
