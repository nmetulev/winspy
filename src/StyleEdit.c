//
//	StyleEdit.c
//
//  Copyright (c) 2002 by J Brown 
//  Freeware
//
//	Implements the Style Editor dialog box
//
//  void ShowWindowStyleEditor(HWND hwndParent, HWND hwndTarget, BOOL fExtended)
//
//  hwndParent - parent window of dialog
//  hwndTarget - target window
//  fExtended  - display standard (FALSE) or extended (TRUE) window styles
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#include "FindTool.h"
#include "WinSpy.h"
#include "resource.h"
#include "Utils.h"

typedef struct
{
	HWND   hwndTarget;	// what window are we looking at??
	BOOL   fExtended;	// Extended (TRUE) or Normal (FALSE)

	DWORD  dwStyle;		// original style

} StyleEditState;

static StyleEditState state;

void FillStyleLists(HWND hwndTarget, HWND hwndStyleList,
	BOOL fAllStyles, DWORD dwStyle);
void FillExStyleLists(HWND hwndTarget, HWND hwndExStyleList,
	BOOL fAllStyles, DWORD dwStyleEx, BOOL fExtControl);

//
//	Define our callback function for the Window Finder Tool
//
UINT CALLBACK StyleEditWndFindProc(HWND hwndTool, UINT uCode, HWND hwnd)
{
	HWND hwndDlg;
	TCHAR szText[120];

	DWORD dwStyle;

	switch (uCode)
	{
	case WFN_END:
		hwndDlg = GetParent(hwndTool);

		if (GetClassLong(state.hwndTarget, GCW_ATOM) == GetClassLong(hwnd, GCW_ATOM))
		{
			if (state.fExtended)
				dwStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			else
				dwStyle = GetWindowLong(hwnd, GWL_STYLE);

			wsprintf(szText, _T("%08X"), dwStyle);

			SetDlgItemText(hwndDlg, IDC_EDIT1, szText);
		}
		else
		{
			wsprintf(szText, _T("Window %08X\n\nUnable to copy this window's styles, \nbecause it belongs to a different class.  "), hwnd);
			MessageBox(hwndDlg, szText, szAppName, MB_OK | MB_ICONINFORMATION);
		}

		break;

	}
	return 0;
}

INT_PTR CALLBACK StyleEditProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static StyleEditState *state;

	HWND hwndList;

	DWORD dwStyle = 0;
	TCHAR szText[32];

	int topindex;
	int caretindex;

	switch (iMsg)
	{
	case WM_INITDIALOG:

		// Passed through in call to DialogBoxParam
		state = (StyleEditState *)lParam;

		if (state->fExtended)
			dwStyle = GetWindowLong(state->hwndTarget, GWL_EXSTYLE);
		else
			dwStyle = GetWindowLong(state->hwndTarget, GWL_STYLE);

		wsprintf(szText, _T("%08X"), dwStyle);
		SetDlgItemText(hwnd, IDC_EDIT1, szText);

		MakeFinderTool(GetDlgItem(hwnd, IDC_DRAGGER), StyleEditWndFindProc);

		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;

	case WM_MEASUREITEM:
		return FunkyList_MeasureItem(hwnd, (UINT)wParam, (MEASUREITEMSTRUCT *)lParam);

	case WM_DRAWITEM:
		if (wParam == IDC_LIST1)
			return FunkyList_DrawItem(hwnd, (UINT)wParam, (DRAWITEMSTRUCT *)lParam);
		else
			return FALSE;

		//if clicked on one of the underlined static controls, then
	//display window info..
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_EDIT1:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				dwStyle = (DWORD)GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

				hwndList = GetDlgItem(hwnd, IDC_LIST1);

				topindex = (int)SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);
				caretindex = (int)SendMessage(hwndList, LB_GETCARETINDEX, 0, 0);

				if (state->fExtended)
					FillExStyleLists(state->hwndTarget, hwndList, TRUE, dwStyle, FALSE);
				else
					FillStyleLists(state->hwndTarget, hwndList, TRUE, dwStyle);

				SendMessage(hwndList, LB_SETCARETINDEX, caretindex, 0);
				SendMessage(hwndList, LB_SETTOPINDEX, topindex, 0);

				return TRUE;
			}

			return FALSE;

		case IDC_APPLY:

			dwStyle = (DWORD)GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

			if (state->fExtended)
				SetWindowLong(state->hwndTarget, GWL_EXSTYLE, dwStyle);
			else
				SetWindowLong(state->hwndTarget, GWL_STYLE, dwStyle);

			SetWindowPos(state->hwndTarget, 0,
				0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
				SWP_NOACTIVATE | SWP_DRAWFRAME);

			InvalidateRect(state->hwndTarget, 0, TRUE);

			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			return TRUE;

		case IDC_CLEAR:
			// I don't know why anyone would use that button
			SetDlgItemText(hwnd, IDC_EDIT1, _T("00000000"));
			return TRUE;

		}

		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			if (LOWORD(wParam) == IDC_LIST1)
			{
				int cursel, caretidx;

				hwndList = GetDlgItem(hwnd, IDC_LIST1);

				dwStyle = (DWORD)GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

				caretidx = (int)SendMessage(hwndList, LB_GETCARETINDEX, 0, 0);
				cursel = (int)SendMessage(hwndList, LB_GETSEL, caretidx, 0);

				if (cursel)
					dwStyle |= SendMessage(hwndList, LB_GETITEMDATA, caretidx, 0);
				else
					dwStyle &= ~SendMessage(hwndList, LB_GETITEMDATA, caretidx, 0);

				wsprintf(szText, _T("%08X"), dwStyle);
				SetDlgItemText(hwnd, IDC_EDIT1, szText);

				return TRUE;
			}

			return FALSE;
		}

		return FALSE;
	}
	return FALSE;
}


void ShowWindowStyleEditor(HWND hwndParent, HWND hwndTarget, BOOL fExtended)
{
	state.hwndTarget = hwndTarget;
	state.dwStyle = 0;
	state.fExtended = fExtended;

	DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_STYLE_EDIT), hwndParent, StyleEditProc, (LPARAM)&state);

	// Update the main display 
	SetGeneralInfo(hwndTarget);
	SetStyleInfo(hwndTarget);
}