//
//  StyleEdit.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Implements the Style Editor dialog box
//
//  void ShowWindowStyleEditor(HWND hwndParent, HWND hwndTarget, BOOL fExtended)
//
//  hwndParent - parent window of dialog
//  hwndTarget - target window
//  fExtended  - display standard (FALSE) or extended (TRUE) window styles
//

#include "WinSpy.h"
#include "FindTool.h"
#include "resource.h"
#include "Utils.h"

typedef struct
{
	HWND   hwndTarget;  // what window are we looking at??
	BOOL   fExtended;   // Extended (TRUE) or Normal (FALSE)

	DWORD  dwStyles;    // original styles; not currently used, but could be used to reset the dialog

} StyleEditState;

static StyleEditState state;

void FillStyleLists(HWND hwndTarget, HWND hwndStyleList,
	BOOL fAllStyles, DWORD dwStyles);
void FillExStyleLists(HWND hwndTarget, HWND hwndExStyleList,
	BOOL fAllStyles, DWORD dwExStyles, BOOL fExtControl);

//
//  Define our callback function for the Window Finder Tool
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

			_stprintf_s(szText, ARRAYSIZE(szText), szHexFmt, dwStyle);

			SetDlgItemText(hwndDlg, IDC_EDIT1, szText);
		}
		else
		{
			_stprintf_s(szText, ARRAYSIZE(szText), _T("Window ") szHexFmt _T("\n\nUnable to copy this window's styles, \nbecause it belongs to a different class.  "), (UINT)(UINT_PTR)hwnd);
			MessageBox(hwndDlg, szText, szAppName, MB_OK | MB_ICONINFORMATION);
		}

		break;

	}
	return 0;
}

INT_PTR CALLBACK StyleEditProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static StyleEditState *pState;

	HWND hwndList;

	DWORD dwStyles;
	TCHAR szText[32];

	int topindex;
	int caretindex;

	switch (iMsg)
	{
	case WM_INITDIALOG:

		// Passed through in call to DialogBoxParam
		pState = (StyleEditState *)lParam;

		if (pState->fExtended)
			dwStyles = GetWindowLong(pState->hwndTarget, GWL_EXSTYLE);
		else
			dwStyles = GetWindowLong(pState->hwndTarget, GWL_STYLE);

		_stprintf_s(szText, ARRAYSIZE(szText), szHexFmt, dwStyles);
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

				if (pState->fExtended)
					FillExStyleLists(pState->hwndTarget, hwndList, TRUE, dwStyles, FALSE);
				else
					FillStyleLists(pState->hwndTarget, hwndList, TRUE, dwStyles);

				SendMessage(hwndList, LB_SETCARETINDEX, caretindex, 0);
				SendMessage(hwndList, LB_SETTOPINDEX, topindex, 0);

				return TRUE;
			}

			return FALSE;

		case IDC_APPLY:

			dwStyles = (DWORD)GetDlgItemBaseInt(hwnd, IDC_EDIT1, 16);

			if (pState->fExtended)
				SetWindowLong(pState->hwndTarget, GWL_EXSTYLE, dwStyles);
			else
				SetWindowLong(pState->hwndTarget, GWL_STYLE, dwStyles);

			SetWindowPos(pState->hwndTarget, 0,
				0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
				SWP_NOACTIVATE | SWP_DRAWFRAME);

			InvalidateRect(pState->hwndTarget, 0, TRUE);

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

				_stprintf_s(szText, ARRAYSIZE(szText), szHexFmt, dwStyles);
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
	state.dwStyles = 0;
	state.fExtended = fExtended;

	DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_STYLE_EDIT), hwndParent, StyleEditProc, (LPARAM)&state);

	// Update the main display
	SetGeneralInfo(hwndTarget);
	SetStyleInfo(hwndTarget);
}