//
//	DisplayPropInfo.c
//  Copyright (c) 2002 by J Brown 
//	Freeware
//
//	void SetPropertyInfo(HWND hwnd)
//
//	Fill the properties-tab-pane with class info for the
//  specified window
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#include "resource.h"
#include "WinSpy.h"

//
//	Called once for each window property
//
BOOL CALLBACK PropEnumProcEx(HWND hwnd, LPTSTR lpszString, HANDLE hData, ULONG_PTR dwUser)
{
	HWND   hwndList = (HWND)dwUser;
	TCHAR  ach[256];
	LVITEM lvitem;
	int    index;

	lvitem.mask = LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem = 0;
	lvitem.iSubItem = 0;
	lvitem.lParam = 0;

	// check that lpszString is a valid string, and not an ATOM in disguise
	if (((ULONG_PTR)lpszString & ~(ULONG_PTR)0xFFFF) == 0)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), szAtomFmt _T(" (Atom)"), (ATOM)lpszString);
		lvitem.pszText = ach;

		lvitem.lParam = (LPARAM)lpszString;
	}
	else
		lvitem.pszText = lpszString;

	index = ListView_InsertItem(hwndList, &lvitem);
	if (index != -1)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, (void*)hData);
		ListView_SetItemText(hwndList, index, 1, ach);
	}

	return TRUE;
}

//
//	Display the window properties (SetProp API)
//
void EnumWindowProps(HWND hwnd, HWND hwndList)
{
	ListView_DeleteAllItems(hwndList);
	EnumPropsEx(hwnd, PropEnumProcEx, (ULONG_PTR)hwndList);
}

void SetPropertyInfo(HWND hwnd)
{
	if (hwnd == 0) return;
	EnumWindowProps(hwnd, GetDlgItem(WinSpyTab[PROPERTY_TAB].hwnd, IDC_LIST1));
}

