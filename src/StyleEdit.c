//
//  StyleEdit.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Implements the Style Editor dialog box
//

#include "WinSpy.h"
#include "FindTool.h"
#include "resource.h"
#include "Utils.h"

typedef struct
{
    HWND            hwndTarget;      // what window are we looking at??
    UINT            flavor;          // STYLE_FLAVOR_
    DWORD           dwStyles;        // Initial value
    ClassStyleInfo* pClassInfo;
}
StyleEditState;

static StyleEditState g_state;


//
//  Define our callback function for the Window Finder Tool
//
UINT CALLBACK StyleEditWndFindProc(HWND hwndTool, UINT uCode, HWND hwnd)
{
    HWND hwndDlg;
    WCHAR szText[120];

    switch (uCode)
    {
    case WFN_END:
        hwndDlg = GetParent(hwndTool);

        if (GetClassLong(g_state.hwndTarget, GCW_ATOM) == GetClassLong(hwnd, GCW_ATOM))
        {
            DWORD dwStyle = 0;
            BOOL fHasValue = FALSE;

            if (g_state.flavor == STYLE_FLAVOR_REGULAR)
            {
                dwStyle = GetWindowLong(hwnd, GWL_STYLE);
                fHasValue = TRUE;
            }
            else if (g_state.flavor == STYLE_FLAVOR_EX)
            {
                dwStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                fHasValue = TRUE;
            }
            else if (GetWindowExtraStyles(hwnd, g_state.pClassInfo, &dwStyle) == ERROR_SUCCESS)
            {
                fHasValue = TRUE;
            }

            if (fHasValue)
            {
                FormatDlgItemText(hwndDlg, IDC_EDIT1, L"%08X", dwStyle);
            }
        }
        else
        {
            swprintf_s(szText, ARRAYSIZE(szText), L"Window %08X\n\nUnable to copy this window's styles, \nbecause it belongs to a different class.  ", (UINT)(UINT_PTR)hwnd);
            MessageBox(hwndDlg, szText, szAppName, MB_OK | MB_ICONINFORMATION);
        }

        break;

    }
    return 0;
}

void ApplyStyle(HWND hwndDlg)
{
    DWORD dwStyles = (DWORD)GetDlgItemBaseInt(hwndDlg, IDC_EDIT1, 16);

    if (g_state.flavor == STYLE_FLAVOR_REGULAR)
    {
        SetWindowLong(g_state.hwndTarget, GWL_STYLE, dwStyles);
    }
    else if (g_state.flavor == STYLE_FLAVOR_EX)
    {
        SetWindowLong(g_state.hwndTarget, GWL_EXSTYLE, dwStyles);
    }
    else
    {
        LRESULT lr;
        DWORD_PTR result;

        lr = SendMessageTimeout(
                g_state.hwndTarget,
                g_state.pClassInfo->SetMessage,
                0,
                dwStyles,
                SMTO_BLOCK | SMTO_ERRORONEXIT,
                100, // 1/10 second
                &result);

        if (!lr)
        {
            return;
        }
    }

    // Force the window to repaint.

    SetWindowPos(
        g_state.hwndTarget,
        0, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);

    InvalidateRect(g_state.hwndTarget, 0, TRUE);
}

INT_PTR CALLBACK StyleEditProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndList;

    DWORD dwStyles;
    WCHAR szText[32];

    int topindex;
    int caretindex;

    switch (iMsg)
    {
    case WM_INITDIALOG:

        FormatDlgItemText(hwnd, IDC_EDIT1, L"%08X", g_state.dwStyles);

        MakeFinderTool(GetDlgItem(hwnd, IDC_DRAGGER), StyleEditWndFindProc);

        return TRUE;

    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;

    case WM_MEASUREITEM:
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FunkyList_MeasureItem(hwnd, (MEASUREITEMSTRUCT *)lParam));
        return TRUE;

    case WM_DRAWITEM:
        if (wParam == IDC_LIST1)
        {
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FunkyList_DrawItem(hwnd, (UINT)wParam, (DRAWITEMSTRUCT *)lParam));
            return TRUE;
        }
        else
            return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_EDIT1:
            switch (HIWORD(wParam))
            {
            case EN_CHANGE:
                dwStyles = (DWORD)GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

                hwndList = GetDlgItem(hwnd, IDC_LIST1);

                topindex = (int)SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);
                caretindex = (int)SendMessage(hwndList, LB_GETCARETINDEX, 0, 0);

                FillStyleListForEditing(g_state.hwndTarget, hwndList, g_state.flavor, dwStyles);

                SendMessage(hwndList, LB_SETCARETINDEX, caretindex, 0);
                SendMessage(hwndList, LB_SETTOPINDEX, topindex, 0);

                return TRUE;
            }

            return FALSE;

        case IDC_APPLY:
            ApplyStyle(hwnd);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;

        case IDC_CLEAR:
            // I don't know why anyone would use that button
            SetDlgItemText(hwnd, IDC_EDIT1, L"00000000");
            return TRUE;

        }

        switch (HIWORD(wParam))
        {
        case LBN_SELCHANGE:
            if (LOWORD(wParam) == IDC_LIST1)
            {
                hwndList = GetDlgItem(hwnd, IDC_LIST1);

                dwStyles = (DWORD)GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

                int caretidx = (int)SendMessage(hwndList, LB_GETCARETINDEX, 0, 0);
                int cursel = (int)SendMessage(hwndList, LB_GETSEL, caretidx, 0);

                StyleLookupEx *pStyle = (StyleLookupEx *)SendMessage(hwndList, LB_GETITEMDATA, caretidx, 0);
                if (cursel)
                {
                    // The user has just selected this item. This means this item has a style definition:
                    // the only one that does not is the "unrecognized bits" item,
                    // and that one is always selected on every repopulation of the list.

                    // If there is a dependency, set the dependency style to be present
                    dwStyles &= ~(pStyle->dependencyValue | pStyle->dependencyExtraMask);
                    dwStyles |= pStyle->dependencyValue;
                    // Now set the style itself to be present
                    dwStyles &= ~(pStyle->value | pStyle->extraMask);
                    dwStyles |= pStyle->value;
                }
                else
                {
                    DWORD style;
                    if (pStyle)
                        style = pStyle->value;
                    else
                    {
                        // This is the "unrecognized bits" item
                        SendMessage(hwndList, LB_GETTEXT, caretidx, (LONG_PTR)szText);
                        style = (DWORD)_tstrtoib16(szText);
                    }
                    dwStyles &= ~style;
                }

                FormatDlgItemText(hwnd, IDC_EDIT1, L"%08X", dwStyles);

                return TRUE;
            }

            return FALSE;
        }

        return FALSE;
    }
    return FALSE;
}


void ShowWindowStyleEditor(HWND hwndParent, HWND hwndTarget, UINT flavor)
{
    g_state.hwndTarget = hwndTarget;
    g_state.flavor     = flavor;
    g_state.pClassInfo = FindClassStyleInfo(hwndTarget);

    // Fetch the initial value.

    if (flavor == STYLE_FLAVOR_REGULAR)
    {
        g_state.dwStyles = GetWindowLong(hwndTarget, GWL_STYLE);
    }
    else if (flavor == STYLE_FLAVOR_EX)
    {
        g_state.dwStyles = GetWindowLong(hwndTarget, GWL_EXSTYLE);
    }
    else if (!g_state.pClassInfo || !g_state.pClassInfo->StylesExtra)
    {
        // Invoked for STYLE_FLAVOR_EXTRA, but we didn't find any class info.
        //
        // This could happen if the HWND was destroyed out from under us,
        // just give up and do nothing.

        return;
    }
    else
    {
        // This is STYLE_FLAVOR_EXTRA.  If we are unable to query the value
        // then don't show the editing dialog.

        DWORD dwErr = GetWindowExtraStyles(
                         hwndTarget,
                         g_state.pClassInfo,
                         &g_state.dwStyles);

        if (dwErr != ERROR_SUCCESS)
        {
            return;
        }
    }

    DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_STYLE_EDIT), hwndParent, StyleEditProc, 0);

    // Update the main display
    UpdateGeneralTab(hwndTarget);
    UpdateStyleTab(hwndTarget);
}
