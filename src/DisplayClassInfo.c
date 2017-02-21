//
//	DisplayClassInfo.c
//  Copyright (c) 2002 by J Brown 
//	Freeware
//
//	void SetClassInfo(HWND hwnd)
//
//	Fill the class-tab-pane with class info for the
//  specified window
//
//	History:
//
//	1.7.1 - fixed bug where 'resolve' window-proc wasn't getting updated
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#include "WinSpy.h"
#include "resource.h"
#include "Utils.h"

void VerboseClassName(TCHAR ach[])
{
	if (lstrcmpi(ach, _T("#32770")) == 0) lstrcat(ach, _T(" (Dialog)"));
	else if (lstrcmpi(ach, _T("#32768")) == 0) lstrcat(ach, _T(" (Menu)"));
	else if (lstrcmpi(ach, _T("#32769")) == 0) lstrcat(ach, _T(" (Desktop window)"));
	else if (lstrcmpi(ach, _T("#32771")) == 0) lstrcat(ach, _T(" (Task-switch window)"));
	else if (lstrcmpi(ach, _T("#32772")) == 0) lstrcat(ach, _T(" (Icon title)"));
}

//	
//	Class styles lookup table
//
StyleLookupType ClassLookup[] =
{
	STYLE_(CS_BYTEALIGNCLIENT),
	STYLE_(CS_BYTEALIGNWINDOW),
	STYLE_(CS_OWNDC),
	STYLE_(CS_CLASSDC),
	STYLE_(CS_PARENTDC),
	STYLE_(CS_DBLCLKS),
	STYLE_(CS_GLOBALCLASS),
	STYLE_(CS_HREDRAW),
	STYLE_(CS_VREDRAW),
	STYLE_(CS_NOCLOSE),
	STYLE_(CS_SAVEBITS),

	STYLE_(CS_IME)
};

//
//	Stock Icon lookup table. These values must be converted to
//	stock icon handle values by calling LoadIcon(NULL, ID) before
//	the list can be searched.
//
HandleLookupType IconLookup[] =
{
	HANDLE_(IDI_WARNING),
	HANDLE_(IDI_ERROR),
	HANDLE_(IDI_INFORMATION),
	HANDLE_(IDI_APPLICATION),
	HANDLE_(IDI_HAND),
	HANDLE_(IDI_QUESTION),
	HANDLE_(IDI_EXCLAMATION),
	HANDLE_(IDI_ASTERISK),

#if(WINVER >= 0x0400)
	HANDLE_(IDI_WINLOGO),
#endif

};

//
//	Stock Cursor lookup table. These values must also be 
//  converted to stock cursor handles.
//
HandleLookupType CursorLookup[] =
{
	HANDLE_(IDC_ARROW),
	HANDLE_(IDC_IBEAM),
	HANDLE_(IDC_WAIT),
	HANDLE_(IDC_CROSS),
	HANDLE_(IDC_UPARROW),
	HANDLE_(IDC_SIZE),
	HANDLE_(IDC_ICON),
	HANDLE_(IDC_SIZENWSE),
	HANDLE_(IDC_SIZENESW),
	HANDLE_(IDC_SIZEWE),
	HANDLE_(IDC_SIZENS),
	HANDLE_(IDC_SIZEALL),
	HANDLE_(IDC_NO),

#if(WINVER >= 0x0500)
	HANDLE_(IDC_HAND),
#endif

	HANDLE_(IDC_APPSTARTING),

#if(WINVER >= 0x0400)
	HANDLE_(IDC_HELP),
#endif

};

//
//	COLOR_xx Brush ID lookup. Needs no conversion
//
StyleLookupType BrushLookup[] =
{
	STYLE_(COLOR_SCROLLBAR),
	STYLE_(COLOR_BACKGROUND),
	STYLE_(COLOR_ACTIVECAPTION),
	STYLE_(COLOR_INACTIVECAPTION),
	STYLE_(COLOR_MENU),
	STYLE_(COLOR_WINDOW),
	STYLE_(COLOR_WINDOWFRAME),
	STYLE_(COLOR_MENUTEXT),
	STYLE_(COLOR_WINDOWTEXT),
	STYLE_(COLOR_CAPTIONTEXT),
	STYLE_(COLOR_ACTIVEBORDER),
	STYLE_(COLOR_INACTIVEBORDER),
	STYLE_(COLOR_APPWORKSPACE),
	STYLE_(COLOR_HIGHLIGHT),
	STYLE_(COLOR_HIGHLIGHTTEXT),
	STYLE_(COLOR_BTNFACE),
	STYLE_(COLOR_BTNSHADOW),
	STYLE_(COLOR_GRAYTEXT),
	STYLE_(COLOR_BTNTEXT),
	STYLE_(COLOR_INACTIVECAPTIONTEXT),
	STYLE_(COLOR_BTNHIGHLIGHT),

#if(WINVER >= 0x0400)
	STYLE_(COLOR_3DDKSHADOW),
	STYLE_(COLOR_3DLIGHT),
	STYLE_(COLOR_INFOTEXT),
	STYLE_(COLOR_INFOBK),
#endif

#if(WINVER >= 0x0500)
	STYLE_(COLOR_HOTLIGHT),
	STYLE_(COLOR_GRADIENTACTIVECAPTION),
	STYLE_(COLOR_GRADIENTINACTIVECAPTION),
#endif

};

//
//	GetStockObject brush lookup. These values must be
//  converted to valid stock brushes.
//
StyleLookupType StkBrLookup[] =
{
	STYLE_(WHITE_BRUSH),
	STYLE_(BLACK_BRUSH),
	STYLE_(LTGRAY_BRUSH),
	STYLE_(GRAY_BRUSH),
	STYLE_(DKGRAY_BRUSH),
	STYLE_(NULL_BRUSH),
};

#define NUM_ICON_LOOKUP ARRAYSIZE(IconLookup)
#define NUM_CURSOR_LOOKUP ARRAYSIZE(CursorLookup)
#define NUM_CLASS_STYLES ARRAYSIZE(ClassLookup)
#define NUM_BRUSH_STYLES ARRAYSIZE(BrushLookup)
#define NUM_STKBR_STYLES ARRAYSIZE(StkBrLookup)

//
//	This table is a combination of the BrushLookup and StkBrLookup tables.
//	All values are handles to stock brushes.
//
HandleLookupType BrushLookup2[NUM_BRUSH_STYLES + NUM_STKBR_STYLES];

#define NUM_BRUSH2_LOOKUP ARRAYSIZE(BrushLookup2)

//
//	Prepare the resource lookup tables by obtaining the 
//  internal handle values for all stock objects.
//
void InitStockStyleLists()
{
	int i;
	for (i = 0; i < NUM_ICON_LOOKUP; i++)
		IconLookup[i].handle = LoadIcon(NULL, MAKEINTRESOURCE(IconLookup[i].handle));

	for (i = 0; i < NUM_CURSOR_LOOKUP; i++)
		CursorLookup[i].handle = LoadCursor(NULL, MAKEINTRESOURCE(CursorLookup[i].handle));

	for (i = 0; i < NUM_BRUSH_STYLES; i++)
	{
		BrushLookup2[i].handle = GetSysColorBrush(BrushLookup[i].style);
		BrushLookup2[i].szName = BrushLookup[i].szName;
	}

	for (i = 0; i < NUM_STKBR_STYLES; i++)
	{
		BrushLookup2[i + NUM_BRUSH_STYLES].handle = GetStockObject(StkBrLookup[i].style);
		BrushLookup2[i + NUM_BRUSH_STYLES].szName = StkBrLookup[i].szName;
	}
}

//
//	Lookup the specified value in the style list
//
int FormatStyle(TCHAR *ach, StyleLookupType *stylelist, int items, UINT matchthis)
{
	int i;

	for (i = 0; i < items; i++)
	{
		if (stylelist[i].style == matchthis)
		{
			lstrcpy(ach, stylelist[i].szName);
			return i;
		}
	}

	return -1;
}

int PrintHandle(TCHAR *ach, ULONG_PTR value)
{
	// if the handle value fits into 32 bits, only use the 32-bit hex format; otherwise full pointer format (this only makes a difference for 64 bit)
	return wsprintf(ach, value <= MAXUINT ? szHexFmt : szPtrFmt, value);
}

//
//	Lookup the specified value in the handle list
//
int FormatHandle(TCHAR *ach, HandleLookupType *handlelist, int items, ULONG_PTR matchthis)
{
	int i;

	for (i = 0; i < items; i++)
	{
		if (handlelist[i].handle == (HANDLE)matchthis)
		{
			lstrcpy(ach, handlelist[i].szName);
			return i;
		}
	}

	if (matchthis == 0 || (HANDLE)matchthis == INVALID_HANDLE_VALUE)
		lstrcpy(ach, _T("(None)"));
	else
		PrintHandle(ach, matchthis);

	return -1;
}

void FillBytesList(
	HWND hwndDlg,
	HWND hwnd,
	int numBytes,
	WORD WINAPI pGetWord(HWND, int),
	LONG WINAPI pGetLong(HWND, int),
	LONG_PTR WINAPI pGetLongPtr(HWND, int)
)
{
	TCHAR ach[256];
	int i = 0;
	LONG_PTR lp;

	SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_RESETCONTENT, 0, 0);
	EnableDlgItem(hwndDlg, IDC_BYTESLIST, numBytes != 0);

	// Retrieve all the bytes + add to combo box
	// We will be getting all the bytes except for the possible last incomplete portion by Get*LongPtr.
	// Because Get*Long* checks the bounds, the last piece has to be retrieved with one operation whose span ends at the very last byte.
	while (numBytes > 0)
	{
		SetLastError(ERROR_SUCCESS);

		// get the biggest chunk (WORD, LONG or LONG_PTR) that will fit in the bytes; if it ends at the last byte, clear out the bytes we don't need
		int chunkBytes, extraBytes;
		// |--------i----------|---numBytes---|
		// |              |     chunkBytes    |
		// |              | eb |
		// or
		// |--------i----------|-------numBytes-------|
		// |-------------------|---chunkBytes-----|
		// eb = 0
		if (i + numBytes >= sizeof(LONG_PTR))
		{
			chunkBytes = sizeof(LONG_PTR);
			extraBytes = max(0, chunkBytes - numBytes);
			lp = pGetLongPtr(hwnd, i - extraBytes);
		}
		else if (i + numBytes >= sizeof(LONG))
		{
			chunkBytes = sizeof(LONG);
			extraBytes = max(0, chunkBytes - numBytes);
			lp = pGetLong(hwnd, i - extraBytes);
		}
		else
		{
			// WORD is the smallest chunk we can access. If there is only 1 byte, we will never be able to set or get that 1 byte.
			chunkBytes = sizeof(WORD);
			extraBytes = max(0, chunkBytes - numBytes);
			if (i < extraBytes)
			{
				chunkBytes -= extraBytes - i;
				extraBytes -= extraBytes - i;
			}
			lp = pGetWord(hwnd, i - extraBytes);
		}

		DWORD dwLastError = GetLastError();
		if (dwLastError == ERROR_PRIVATE_DIALOG_INDEX)
			break;

		// by this point, the lowest chunkBytes of lp contain the data retrieved; the lowest extraBytes of them are overlapping previously retrieved data
#define bitsInByte 8
		lp >>= bitsInByte * extraBytes;
		chunkBytes -= extraBytes;
		// by this point, the lowest chunkBytes of lp contain the data needed. Clear all higher bytes
		if (chunkBytes < sizeof(lp))
			lp &= (1ll << bitsInByte * chunkBytes) - 1;

		if (dwLastError == ERROR_SUCCESS)
		{
			_stprintf_s(ach, ARRAYSIZE(ach), _T("+%-8d %0*IX"), i, 2 * chunkBytes, lp);
		}
		else
			_stprintf_s(ach, ARRAYSIZE(ach), _T("+%-8d Unavailable (0x%08X)"), i, dwLastError);

		i += chunkBytes;
		numBytes -= chunkBytes;

		LRESULT index = SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_ADDSTRING, 0, (LPARAM)ach);
		SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_SETITEMDATA, index, dwLastError == ERROR_SUCCESS ? lp : dwLastError);
	}

	SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_SETCURSEL, 0, 0);
}

//
//	Set the class information on the Class Tab, for the specified window
//
void SetClassInfo(HWND hwnd)
{
	TCHAR ach[256];

	int i, numstyles, classbytes;
	HWND hwndDlg = WinSpyTab[CLASS_TAB].hwnd;
	UINT_PTR handle;

	if (hwnd == 0) return;


	GetClassName(hwnd, ach, ARRAYSIZE(ach));

	// be nice and give the proper name for the following class names
	//
	VerboseClassName(ach);

	SetDlgItemText(hwndDlg, IDC_CLASSNAME, ach);

	//class style
	wsprintf(ach, szHexFmt, spy_WndClassEx.style);
	SetDlgItemText(hwndDlg, IDC_STYLE, ach);

	//atom
	wsprintf(ach, _T("%04X"), GetClassLong(hwnd, GCW_ATOM));
	SetDlgItemText(hwndDlg, IDC_ATOM, ach);

	//extra class bytes
	wsprintf(ach, _T("%d"), spy_WndClassEx.cbClsExtra);
	SetDlgItemText(hwndDlg, IDC_CLASSBYTES, ach);

	//extra window bytes
	wsprintf(ach, _T("%d"), spy_WndClassEx.cbWndExtra);
	SetDlgItemText(hwndDlg, IDC_WINDOWBYTES, ach);

	//menu (not implemented)
	wsprintf(ach, szPtrFmt, GetClassLongPtr(hwnd, GCLP_MENUNAME));
	SetDlgItemText(hwndDlg, IDC_MENUHANDLE, _T("(None)"));

	//cursor handle
	handle = GetClassLongPtr(hwnd, GCLP_HCURSOR);
	FormatHandle(ach, CursorLookup, NUM_CURSOR_LOOKUP, handle);
	SetDlgItemText(hwndDlg, IDC_CURSORHANDLE, ach);

	//icon handle
	handle = GetClassLongPtr(hwnd, GCLP_HICON);
	FormatHandle(ach, IconLookup, NUM_ICON_LOOKUP, handle);
	SetDlgItemText(hwndDlg, IDC_ICONHANDLE, ach);

	//background brush handle
	handle = GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND);

	//see hbrBackground description at https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577.aspx
	//first of all, search by COLOR_xxx value
	if (!(handle > 0 && handle <= MAXUINT) || (-1 == FormatStyle(ach, BrushLookup, NUM_BRUSH_STYLES, (UINT)handle - 1)))
	{
		//now search by handle value
		i = FormatHandle(ach, BrushLookup2, NUM_BRUSH2_LOOKUP, handle);
		if (i != -1)
		{
			wsprintf(ach + PrintHandle(ach, (UINT_PTR)BrushLookup2[i].handle), _T("  (%s)"), BrushLookup2[i].szName);
		}
	}

	//set the brush handle text
	SetDlgItemText(hwndDlg, IDC_BKGNDBRUSH, ach);

	//window procedure
	if (spy_WndProc == 0)
	{
		wsprintf(ach, _T("N/A"));
	}
	else
	{
		wsprintf(ach, szPtrFmt, spy_WndProc);
		if (spy_WndProc != spy_WndClassEx.lpfnWndProc)
			lstrcat(ach, _T(" (Subclassed)"));
	}

	SetDlgItemText(hwndDlg, IDC_WNDPROC, ach);

	SetDlgItemText(WinSpyTab[GENERAL_TAB].hwnd, IDC_WINDOWPROC2, ach);

	//class window procedure
	if (spy_WndClassEx.lpfnWndProc == 0)
		wsprintf(ach, _T("N/A"));
	else
		wsprintf(ach, szPtrFmt, spy_WndClassEx.lpfnWndProc);

	SetDlgItemText(hwndDlg, IDC_CLASSPROC, ach);



	//instance handle
	wsprintf(ach, szPtrFmt, spy_WndClassEx.hInstance);
	SetDlgItemText(hwndDlg, IDC_INSTANCEHANDLE, ach);

	//
	// fill the combo box with the class styles
	//
	numstyles = 0;
	SendDlgItemMessage(hwndDlg, IDC_STYLELIST, CB_RESETCONTENT, 0, 0);
	for (i = 0; i < NUM_CLASS_STYLES; i++)
	{
		if (spy_WndClassEx.style & ClassLookup[i].style)
		{
			SendDlgItemMessage(hwndDlg, IDC_STYLELIST, CB_ADDSTRING, 0,
				(LPARAM)ClassLookup[i].szName);

			numstyles++;
		}
	}

	SendDlgItemMessage(hwndDlg, IDC_STYLELIST, CB_SETCURSEL, 0, 0);
	EnableDlgItem(hwndDlg, IDC_STYLELIST, numstyles != 0);

	//
	// fill combo box with class extra bytes
	//
	classbytes = spy_WndClassEx.cbClsExtra;

	FillBytesList(hwndDlg, hwnd, classbytes, GetClassWord, GetClassLong, GetClassLongPtr);
}

