//
//  Options.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Implements the Options dialog for WinSpy
//

#include "WinSpy.h"
#include "RegHelper.h"
#include "resource.h"

Options g_opts;

extern HWND hwndToolTip;

#define REG_BASESTR  _T("Software\\Catch22\\WinSpy++ 1.5")

static TCHAR szRegLoc[] = REG_BASESTR;

void LoadSettings(void)
{
    HKEY hkey;

    RegCreateKeyEx(HKEY_CURRENT_USER, szRegLoc, 0, _T(""), 0, KEY_READ, NULL, &hkey, NULL);

    g_opts.fSaveWinPos = GetSettingBool(hkey, _T("SavePosition"), TRUE);
    g_opts.fAlwaysOnTop = GetSettingBool(hkey, _T("AlwaysOnTop"), FALSE);
    g_opts.fMinimizeWinSpy = GetSettingBool(hkey, _T("MinimizeWinSpy"), TRUE);
    g_opts.fFullDragging = GetSettingBool(hkey, _T("FullDragging"), TRUE);
    g_opts.fShowHidden = GetSettingBool(hkey, _T("ShowHidden"), FALSE);
    g_opts.fShowDimmed = GetSettingBool(hkey, _T("ShowDimmed"), TRUE);
    g_opts.fClassThenText = GetSettingBool(hkey, _T("ClassThenText"), TRUE);
    g_opts.fPinWindow = GetSettingBool(hkey, _T("PinWindow"), FALSE);
    g_opts.fShowInCaption = GetSettingBool(hkey, _T("ShowInCaption"), TRUE);
    g_opts.fEnableToolTips = GetSettingBool(hkey, _T("EnableToolTips"), FALSE);
    g_opts.fShowDesktopRoot = GetSettingBool(hkey, _T("ShowDesktopRoot"), FALSE);
    g_opts.uTreeInclude = GetSettingInt(hkey, _T("TreeItems"), WINLIST_INCLUDE_ALL);
    g_opts.fShowHiddenInList = GetSettingBool(hkey, _T("List_ShowHidden"), TRUE);

    g_opts.uPinnedCorner = GetSettingInt(hkey, _T("PinCorner"), 0);

    g_opts.ptPinPos.x = GetSettingInt(hkey, _T("xpos"), CW_USEDEFAULT);
    g_opts.ptPinPos.y = GetSettingInt(hkey, _T("ypos"), CW_USEDEFAULT);

    RegCloseKey(hkey);
}

void SaveSettings(void)
{
    HKEY hkey;

    RegCreateKeyEx(HKEY_CURRENT_USER, szRegLoc, 0, _T(""), 0, KEY_WRITE, NULL, &hkey, NULL);

    WriteSettingBool(hkey, _T("SavePosition"), g_opts.fSaveWinPos);
    WriteSettingBool(hkey, _T("AlwaysOnTop"), g_opts.fAlwaysOnTop);
    WriteSettingBool(hkey, _T("MinimizeWinSpy"), g_opts.fMinimizeWinSpy);
    WriteSettingBool(hkey, _T("FullDragging"), g_opts.fFullDragging);
    WriteSettingBool(hkey, _T("ShowHidden"), g_opts.fShowHidden);
    WriteSettingBool(hkey, _T("ShowDimmed"), g_opts.fShowDimmed);
    WriteSettingBool(hkey, _T("ClassThenText"), g_opts.fClassThenText);
    WriteSettingBool(hkey, _T("PinWindow"), g_opts.fPinWindow);
    WriteSettingBool(hkey, _T("ShowInCaption"), g_opts.fShowInCaption);
    WriteSettingBool(hkey, _T("EnableToolTips"), g_opts.fEnableToolTips);
    WriteSettingBool(hkey, _T("ShowDesktopRoot"), g_opts.fShowDesktopRoot);
    WriteSettingInt(hkey, _T("TreeItems"), g_opts.uTreeInclude);
    WriteSettingInt(hkey, _T("PinCorner"), g_opts.uPinnedCorner);
    WriteSettingInt(hkey, _T("List_ShowHidden"), g_opts.fShowHiddenInList);

    WriteSettingInt(hkey, _T("xpos"), g_opts.ptPinPos.x);
    WriteSettingInt(hkey, _T("ypos"), g_opts.ptPinPos.y);

    RegCloseKey(hkey);
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HWND hwndTarget;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, IDC_OPTIONS_SAVEPOS, g_opts.fSaveWinPos);
        CheckDlgButton(hwnd, IDC_OPTIONS_FULLDRAG, g_opts.fFullDragging);
        CheckDlgButton(hwnd, IDC_OPTIONS_DIR, g_opts.fClassThenText);
        CheckDlgButton(hwnd, IDC_OPTIONS_SHOWHIDDEN, g_opts.fShowDimmed);
        CheckDlgButton(hwnd, IDC_OPTIONS_SHOWINCAPTION, g_opts.fShowInCaption);
        CheckDlgButton(hwnd, IDC_OPTIONS_TOOLTIPS, g_opts.fEnableToolTips);
        CheckDlgButton(hwnd, IDC_OPTIONS_DESKTOPROOT, g_opts.fShowDesktopRoot);
        CheckDlgButton(hwnd, IDC_OPTIONS_LIST_SHOWHIDDEN, g_opts.fShowHiddenInList);

        CheckDlgButton(hwnd, IDC_OPTIONS_INCHANDLE,
            (g_opts.uTreeInclude & WINLIST_INCLUDE_HANDLE) ? TRUE : FALSE);

        CheckDlgButton(hwnd, IDC_OPTIONS_INCCLASS,
            (g_opts.uTreeInclude & WINLIST_INCLUDE_CLASS) ? TRUE : FALSE);

        return TRUE;

    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:

            g_opts.fSaveWinPos = IsDlgButtonChecked(hwnd, IDC_OPTIONS_SAVEPOS);
            g_opts.fFullDragging = IsDlgButtonChecked(hwnd, IDC_OPTIONS_FULLDRAG);
            g_opts.fClassThenText = IsDlgButtonChecked(hwnd, IDC_OPTIONS_DIR);
            g_opts.fShowDimmed = IsDlgButtonChecked(hwnd, IDC_OPTIONS_SHOWHIDDEN);
            g_opts.fShowInCaption = IsDlgButtonChecked(hwnd, IDC_OPTIONS_SHOWINCAPTION);
            g_opts.fEnableToolTips = IsDlgButtonChecked(hwnd, IDC_OPTIONS_TOOLTIPS);
            g_opts.fShowDesktopRoot = IsDlgButtonChecked(hwnd, IDC_OPTIONS_DESKTOPROOT);
            g_opts.fShowHiddenInList = IsDlgButtonChecked(hwnd, IDC_OPTIONS_LIST_SHOWHIDDEN);

            g_opts.uTreeInclude = 0;

            if (IsDlgButtonChecked(hwnd, IDC_OPTIONS_INCHANDLE))
                g_opts.uTreeInclude |= WINLIST_INCLUDE_HANDLE;

            if (IsDlgButtonChecked(hwnd, IDC_OPTIONS_INCCLASS))
                g_opts.uTreeInclude |= WINLIST_INCLUDE_CLASS;

            EndDialog(hwnd, 0);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }

        return FALSE;
    }

    return FALSE;
}


void ShowOptionsDlg(HWND hwndParent)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_OPTIONS), hwndParent, OptionsDlgProc);

    UpdateMainWindowText();

    SendMessage(hwndToolTip, TTM_ACTIVATE, g_opts.fEnableToolTips, 0);
}