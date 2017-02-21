//
//	DisplayGeneralInfo.c
//  Copyright (c) 2002 by J Brown 
//	Freeware
//
//	void SetGeneralInfo(HWND hwnd)
//
//	Fill the general-tab-pane with general info for the
//  specified window
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#include "resource.h"
#include "WinSpy.h"
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
	TCHAR	ach[256];
	HWND	hwndDlg = WinSpyTab[GENERAL_TAB].hwnd;
	RECT	rect;
	int		x1, y1;
	int		numbytes;
	LONG_PTR lp;

	if (hwnd == 0) return;

	//handle
	wsprintf(ach, szHexFmt, hwnd);
	SetDlgItemText(hwndDlg, IDC_HANDLE, ach);

	//caption
	ShowDlgItem(hwndDlg, IDC_CAPTION1, SW_SHOW);
	ShowDlgItem(hwndDlg, IDC_CAPTION2, SW_HIDE);

	SendDlgItemMessage(hwndDlg, IDC_CAPTION2, CB_RESETCONTENT, 0, 0);

	if (!IsWindow(hwnd))
	{
		SetDlgItemText(hwndDlg, IDC_CAPTION1, L"(invalid window)");	// edit box
		SetDlgItemText(hwndDlg, IDC_CAPTION2, L"(invalid window)");	// combo box
		SetDlgItemText(hwndDlg, IDC_CLASS, L"(invalid window)");
		return;
	}

	// SendMessage is better than GetWindowText, 
	// because it gets text of children in other processes
	if (spy_fPassword == FALSE)
	{
		ach[0] = 0;

		if (!SendMessageTimeout(hwnd, WM_GETTEXT, ARRAYSIZE(ach), (LPARAM)ach,
			SMTO_ABORTIFHUNG, 100, NULL))
		{
			GetWindowText(hwnd, ach, ARRAYSIZE(ach));
		}

		SetDlgItemText(hwndDlg, IDC_CAPTION1, ach);	// edit box
		SetDlgItemText(hwndDlg, IDC_CAPTION2, ach);	// combo box
	}
	else
	{
		SetDlgItemText(hwndDlg, IDC_CAPTION1, spy_szPassword);	// edit box
		SetDlgItemText(hwndDlg, IDC_CAPTION2, spy_szPassword);	// combo box
	}

	//class name
	GetClassName(hwnd, ach, ARRAYSIZE(ach));

	if (IsWindowUnicode(hwnd))	lstrcat(ach, _T("  (Unicode)"));

	SetDlgItemText(hwndDlg, IDC_CLASS, ach);

	//style
	wsprintf(ach, szHexFmt, GetWindowLong(hwnd, GWL_STYLE));

	lstrcat(ach, IsWindowVisible(hwnd) ? _T("  (visible, ") : _T("  (hidden, "));

	lstrcat(ach, IsWindowEnabled(hwnd) ? _T("enabled)") : _T("disabled)"));

	SetDlgItemText(hwndDlg, IDC_STYLE, ach);

	//rectangle
	GetWindowRect(hwnd, &rect);
	x1 = rect.left;
	y1 = rect.top;

	wsprintf(ach, _T("(%d,%d) - (%d,%d)  -  %dx%d"),
		rect.left, rect.top, rect.right, rect.bottom,
		(rect.right - rect.left), (rect.bottom - rect.top));

	SetDlgItemText(hwndDlg, IDC_RECTANGLE, ach);

	//client rect
	GetClientRect(hwnd, &rect);
	MapWindowPoints(hwnd, 0, (POINT *)&rect, 2);
	x1 = rect.left - x1;
	y1 = rect.top - y1;

	OffsetRect(&rect, -rect.left, -rect.top);
	OffsetRect(&rect, x1, y1);

	wsprintf(ach, _T("(%d,%d) - (%d,%d)  -  %dx%d"),
		rect.left, rect.top, rect.right, rect.bottom,
		(rect.right - rect.left), (rect.bottom - rect.top));

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
	if (spy_WndProc == 0)
	{
		wsprintf(ach, _T("N/A"));
		SetDlgItemText(hwndDlg, IDC_WINDOWPROC, ach);

		ShowDlgItem(hwndDlg, IDC_WINDOWPROC, SW_SHOW);
		ShowDlgItem(hwndDlg, IDC_WINDOWPROC2, SW_HIDE);
	}
	else
	{
		wsprintf(ach, szPtrFmt, spy_WndProc);
		SetDlgItemText(hwndDlg, IDC_WINDOWPROC2, ach);

		ShowDlgItem(hwndDlg, IDC_WINDOWPROC, SW_HIDE);
		ShowDlgItem(hwndDlg, IDC_WINDOWPROC2, SW_SHOW);
	}

	//instance handle
	wsprintf(ach, szPtrFmt, GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
	SetDlgItemText(hwndDlg, IDC_INSTANCE, ach);

	//user data
	wsprintf(ach, szPtrFmt, GetWindowLongPtr(hwnd, GWLP_USERDATA));
	SetDlgItemText(hwndDlg, IDC_USERDATA, ach);

	//control ID
	lp = GetWindowLongPtr(hwnd, GWLP_ID);
	wsprintf(ach, _T("%p  (%Id)"), lp, lp);
	SetDlgItemText(hwndDlg, IDC_CONTROLID, ach);

	//extra window bytes
	numbytes = GetClassLong(hwnd, GCL_CBWNDEXTRA);

	FillBytesList(hwndDlg, hwnd, numbytes, GetWindowWord, GetWindowLong, GetWindowLongPtr);
}
