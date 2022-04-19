//
//  DisplayGeneralInfo.c
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  void SetGeneralInfo(HWND hwnd)
//
//  Fill the general-tab-pane with general info for the
//  specified window
//

#include "WinSpy.h"

#include "resource.h"
#include "Utils.h"

void RemoveHyperlink(HWND hwnd, UINT staticid);
void MakeHyperlink(HWND hwnd, UINT staticid, COLORREF crLink);
void FillBytesList(
    HWND hwndDlg,
    HWND hwnd,
    int numBytes,
    WORD WINAPI pGetWord(HWND, int),
    LONG WINAPI pGetLong(HWND, int),
    LONG_PTR WINAPI pGetLongPtr(HWND, int)
);

void SetGeneralInfo(HWND hwnd)
{
    TCHAR   ach[256];
    HWND    hwndDlg = WinSpyTab[GENERAL_TAB].hwnd;
    RECT    rect;
    int     x1 = 0, y1 = 0;
    int     numbytes;
    LONG_PTR lp;

    *ach = 0;
    ZeroMemory(&rect, sizeof(rect));

    //handle
    if (hwnd)
    {
        _stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, (UINT)(UINT_PTR)hwnd);
    }
    SetDlgItemText(hwndDlg, IDC_HANDLE, ach);

    BOOL fValid = hwnd != NULL;
    if (hwnd && !IsWindow(hwnd))
    {
        fValid = FALSE;
        _tcscpy_s(ach, ARRAYSIZE(ach), szInvalidWindow);
    }

    //caption
    ShowDlgItem(hwndDlg, IDC_CAPTION1, SW_SHOW);
    ShowDlgItem(hwndDlg, IDC_CAPTION2, SW_HIDE);

    SendDlgItemMessage(hwndDlg, IDC_CAPTION2, CB_RESETCONTENT, 0, 0);

    if (fValid)
    {
        // SendMessage is better than GetWindowText,
        // because it gets text of children in other processes
        if (spy_fPassword)
        {
            // In this case, the password has already been extracted by GetRemoteWindowInfo()
            _tcscpy_s(ach, ARRAYSIZE(ach), spy_szPassword);
        }
        else
        {
            ach[0] = 0;

            if (!SendMessageTimeout(hwnd, WM_GETTEXT, ARRAYSIZE(ach), (LPARAM)ach,
                SMTO_ABORTIFHUNG, 100, NULL))
            {
                GetWindowText(hwnd, ach, ARRAYSIZE(ach));
            }
        }
    }
    SetDlgItemText(hwndDlg, IDC_CAPTION1, ach); // edit box
    SetDlgItemText(hwndDlg, IDC_CAPTION2, ach); // combo box

    //class name
    if (fValid)
    {
        GetClassName(hwnd, ach, ARRAYSIZE(ach));

        VerboseClassName(ach, ARRAYSIZE(ach), (WORD)GetClassLong(hwnd, GCW_ATOM));

        if (IsWindowUnicode(hwnd))
            _tcscat_s(ach, ARRAYSIZE(ach), _T("  (Unicode)"));
    }
    SetDlgItemText(hwndDlg, IDC_CLASS, ach);

    //style
    if (fValid)
    {
        _stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, GetWindowLong(hwnd, GWL_STYLE));
        _tcscat_s(ach, ARRAYSIZE(ach), IsWindowVisible(hwnd) ? _T("  (visible, ") : _T("  (hidden, "));
        _tcscat_s(ach, ARRAYSIZE(ach), IsWindowEnabled(hwnd) ? _T("enabled") : _T("disabled"));

        DWORD dwCloaked = 0;
        DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &dwCloaked, sizeof(dwCloaked));

        if (dwCloaked & DWM_CLOAKED_APP)
        {
            _tcscat_s(ach, ARRAYSIZE(ach), _T(", app cloaked"));
        }
        else if (dwCloaked & DWM_CLOAKED_SHELL)
        {
            _tcscat_s(ach, ARRAYSIZE(ach), _T(", cloaked"));
        }

        _tcscat_s(ach, ARRAYSIZE(ach), _T(")"));
    }
    SetDlgItemText(hwndDlg, IDC_STYLE, ach);

    //rectangle
    if (fValid)
    {
        GetWindowRect(hwnd, &rect);
        x1 = rect.left;
        y1 = rect.top;

        _stprintf_s(ach, ARRAYSIZE(ach), _T("(%d,%d) - (%d,%d)  -  %dx%d"),
            rect.left, rect.top, rect.right, rect.bottom,
            GetRectWidth(&rect), GetRectHeight(&rect));
    }
    SetDlgItemText(hwndDlg, IDC_RECTANGLE, ach);

    //client rect
    if (fValid)
    {
        RECT rcClient;

        GetClientRect(hwnd, &rcClient);
        MapWindowPoints(hwnd, 0, (POINT *)&rcClient, 2);

        if (!g_fShowClientRectAsMargins)
        {
            x1 = rcClient.left - x1;
            y1 = rcClient.top - y1;

            OffsetRect(&rcClient, -rcClient.left, -rcClient.top);
            OffsetRect(&rcClient, x1, y1);

            _stprintf_s(ach, ARRAYSIZE(ach), _T("(%d,%d) - (%d,%d)  -  %dx%d"),
                rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
                GetRectWidth(&rcClient), GetRectHeight(&rcClient));
        }
        else
        {
            _stprintf_s(ach, ARRAYSIZE(ach), _T("{ %d, %d, %d, %d }"),
                rcClient.left - rect.left,
                rcClient.top - rect.top,
                rect.right - rcClient.right,
                rect.bottom - rcClient.bottom);
        }
    }
    SetDlgItemText(hwndDlg, IDC_CLIENTRECT, ach);

    //restored rect
    /*GetWindowPlacement(hwnd, &wp);
    wsprintf(ach, _T("(%d,%d) - (%d,%d)  -  %dx%d"),
        wp.rcNormalPosition.left, wp.rcNormalPosition.top,
        wp.rcNormalPosition.right, wp.rcNormalPosition.bottom,
        (wp.rcNormalPosition.right-wp.rcNormalPosition.left),
        (wp.rcNormalPosition.bottom-wp.rcNormalPosition.top));

    SetDlgItemText(hwndDlg, IDC_RESTOREDRECT, ach);*/

    //window procedure
    if (fValid)
    {
        if (spy_WndProc == 0)
        {
            _stprintf_s(ach, ARRAYSIZE(ach), _T("N/A"));
        }
        else
        {
            _stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, spy_WndProc);
        }
    }
    ShowDlgItem(hwndDlg, IDC_WINDOWPROC, (!fValid || spy_WndProc) ? SW_HIDE : SW_SHOW);
    ShowDlgItem(hwndDlg, IDC_WINDOWPROC2, (!fValid || spy_WndProc) ? SW_SHOW : SW_HIDE);
    SetDlgItemText(hwndDlg, IDC_WINDOWPROC, ach);
    SetDlgItemText(hwndDlg, IDC_WINDOWPROC2, ach);

    //instance handle
    if (fValid)
    {
        _stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, (void*)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
    }
    SetDlgItemText(hwndDlg, IDC_INSTANCE, ach);

    //user data
    if (fValid)
    {
        _stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, (void*)GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    SetDlgItemText(hwndDlg, IDC_USERDATA, ach);

    //control ID
    if (fValid)
    {
        lp = GetWindowLongPtr(hwnd, GWLP_ID);
        // despite the name "GWLP_ID" suggesting that control ID is pointer-sized,
        // it would only work properly in WM_COMMAND if it was a WORD,
        // as it is passed in LOWORD(wParam)
        _stprintf_s(ach, ARRAYSIZE(ach), _T("%04IX  (%Id)"), lp, lp);
    }
    SetDlgItemText(hwndDlg, IDC_CONTROLID, ach);

    //extra window bytes
    numbytes = fValid ? GetClassLong(hwnd, GCL_CBWNDEXTRA) : 0;

    FillBytesList(hwndDlg, hwnd, numbytes, GetWindowWord, GetWindowLong, GetWindowLongPtr);
}
