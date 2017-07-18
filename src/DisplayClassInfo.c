//
//  DisplayClassInfo.c
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  void SetClassInfo(HWND hwnd)
//
//  Fill the class-tab-pane with class info for the
//  specified window
//
//  History:
//
//  1.7.1 - fixed bug where 'resolve' window-proc wasn't getting updated
//

#include "WinSpy.h"

#include "resource.h"
#include "Utils.h"

void VerboseClassName(TCHAR ach[], size_t cch, WORD atom)
{
	// See https://msdn.microsoft.com/en-us/library/windows/desktop/ms633574(v=vs.85).aspx
	if (atom == (intptr_t)WC_DIALOG) _tcscat_s(ach, cch, _T(" (Dialog)"));
	else if (atom == 32768) _tcscat_s(ach, cch, _T(" (Menu)"));
	else if (atom == 32769) _tcscat_s(ach, cch, _T(" (Desktop window)"));
	else if (atom == 32771) _tcscat_s(ach, cch, _T(" (Task-switch window)"));
	else if (atom == 32772) _tcscat_s(ach, cch, _T(" (Icon title)"));
}

//
//  Class styles lookup table
//
ConstLookupType ClassLookup[] =
{
	NAMEANDVALUE_(CS_VREDRAW),
	NAMEANDVALUE_(CS_HREDRAW),
	NAMEANDVALUE_(CS_DBLCLKS),
	NAMEANDVALUE_(CS_OWNDC),
	NAMEANDVALUE_(CS_CLASSDC),
	NAMEANDVALUE_(CS_PARENTDC),
	NAMEANDVALUE_(CS_NOCLOSE),
	NAMEANDVALUE_(CS_SAVEBITS),
	NAMEANDVALUE_(CS_BYTEALIGNCLIENT),
	NAMEANDVALUE_(CS_BYTEALIGNWINDOW),
	NAMEANDVALUE_(CS_GLOBALCLASS),
	NAMEANDVALUE_(CS_IME),
	NAMEANDVALUE_(CS_DROPSHADOW),
};

//
//  Stock Icon lookup table. These values must be converted to
//  stock icon handle values by calling LoadIcon(NULL, ID) before
//  the list can be searched.
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
	HANDLE_(IDI_WINLOGO),
};

//
//  Stock Cursor lookup table. These values must also be
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
	HANDLE_(IDC_HAND),
	HANDLE_(IDC_APPSTARTING),
	HANDLE_(IDC_HELP),
};

//
//  COLOR_xx Brush ID lookup. Needs no conversion
//
ConstLookupType BrushLookup[] =
{
	NAMEANDVALUE_(COLOR_SCROLLBAR),
	NAMEANDVALUE_(COLOR_BACKGROUND),
	NAMEANDVALUE_(COLOR_ACTIVECAPTION),
	NAMEANDVALUE_(COLOR_INACTIVECAPTION),
	NAMEANDVALUE_(COLOR_MENU),
	NAMEANDVALUE_(COLOR_WINDOW),
	NAMEANDVALUE_(COLOR_WINDOWFRAME),
	NAMEANDVALUE_(COLOR_MENUTEXT),
	NAMEANDVALUE_(COLOR_WINDOWTEXT),
	NAMEANDVALUE_(COLOR_CAPTIONTEXT),
	NAMEANDVALUE_(COLOR_ACTIVEBORDER),
	NAMEANDVALUE_(COLOR_INACTIVEBORDER),
	NAMEANDVALUE_(COLOR_APPWORKSPACE),
	NAMEANDVALUE_(COLOR_HIGHLIGHT),
	NAMEANDVALUE_(COLOR_HIGHLIGHTTEXT),
	NAMEANDVALUE_(COLOR_BTNFACE),
	NAMEANDVALUE_(COLOR_BTNSHADOW),
	NAMEANDVALUE_(COLOR_GRAYTEXT),
	NAMEANDVALUE_(COLOR_BTNTEXT),
	NAMEANDVALUE_(COLOR_INACTIVECAPTIONTEXT),
	NAMEANDVALUE_(COLOR_BTNHIGHLIGHT),
	NAMEANDVALUE_(COLOR_3DDKSHADOW),
	NAMEANDVALUE_(COLOR_3DLIGHT),
	NAMEANDVALUE_(COLOR_INFOTEXT),
	NAMEANDVALUE_(COLOR_INFOBK),
	NAMEANDVALUE_(COLOR_HOTLIGHT),
	NAMEANDVALUE_(COLOR_GRADIENTACTIVECAPTION),
	NAMEANDVALUE_(COLOR_GRADIENTINACTIVECAPTION),
};

//
//  GetStockObject brush lookup. These values must be
//  converted to valid stock brushes.
//
ConstLookupType StkBrLookup[] =
{
	NAMEANDVALUE_(WHITE_BRUSH),
	NAMEANDVALUE_(BLACK_BRUSH),
	NAMEANDVALUE_(LTGRAY_BRUSH),
	NAMEANDVALUE_(GRAY_BRUSH),
	NAMEANDVALUE_(DKGRAY_BRUSH),
	NAMEANDVALUE_(NULL_BRUSH),
};

#define NUM_ICON_LOOKUP ARRAYSIZE(IconLookup)
#define NUM_CURSOR_LOOKUP ARRAYSIZE(CursorLookup)
#define NUM_CLASS_STYLES ARRAYSIZE(ClassLookup)
#define NUM_BRUSH_STYLES ARRAYSIZE(BrushLookup)
#define NUM_STKBR_STYLES ARRAYSIZE(StkBrLookup)

//
//  This table is a combination of the BrushLookup and StkBrLookup tables.
//  All values are handles to stock brushes.
//
HandleLookupType BrushLookup2[NUM_BRUSH_STYLES + NUM_STKBR_STYLES];

#define NUM_BRUSH2_LOOKUP ARRAYSIZE(BrushLookup2)

//
//  Prepare the resource lookup tables by obtaining the
//  internal handle values for all stock objects.
//
void InitStockStyleLists()
{
	int i;
	for (i = 0; i < NUM_ICON_LOOKUP; i++)
		IconLookup[i].handle = LoadIcon(NULL, MAKEINTRESOURCE((intptr_t)IconLookup[i].handle));

	for (i = 0; i < NUM_CURSOR_LOOKUP; i++)
		CursorLookup[i].handle = LoadCursor(NULL, MAKEINTRESOURCE((intptr_t)CursorLookup[i].handle));

	for (i = 0; i < NUM_BRUSH_STYLES; i++)
	{
		BrushLookup2[i].handle = GetSysColorBrush(BrushLookup[i].value);
		BrushLookup2[i].szName = BrushLookup[i].szName;
	}

	for (i = 0; i < NUM_STKBR_STYLES; i++)
	{
		BrushLookup2[i + NUM_BRUSH_STYLES].handle = GetStockObject(StkBrLookup[i].value);
		BrushLookup2[i + NUM_BRUSH_STYLES].szName = StkBrLookup[i].szName;
	}
}

//
//  Lookup the specified value in the lookup list
//
int FormatConst(TCHAR *ach, size_t cch, ConstLookupType *list, int items, UINT matchthis)
{
	int i;

	for (i = 0; i < items; i++)
	{
		if (list[i].value == matchthis)
		{
			_tcscpy_s(ach, cch, list[i].szName);
			return i;
		}
	}

	return -1;
}

int PrintHandle(TCHAR *ach, size_t cch, ULONG_PTR value)
{
	// if the handle value fits into 32 bits, only use the 32-bit hex format; otherwise full pointer format (this only makes a difference for 64 bit)
	return _stprintf_s(ach, cch, value <= MAXUINT ? szHexFmt : szPtrFmt, value);
}

//
//  Lookup the specified value in the handle list
//
int FormatHandle(TCHAR *ach, size_t cch, HandleLookupType *handlelist, int items, ULONG_PTR matchthis)
{
	int i;

	for (i = 0; i < items; i++)
	{
		if (handlelist[i].handle == (HANDLE)matchthis)
		{
			_tcscpy_s(ach, cch, handlelist[i].szName);
			return i;
		}
	}

	if (matchthis == 0 || (HANDLE)matchthis == INVALID_HANDLE_VALUE)
		_tcscpy_s(ach, cch, _T("(None)"));
	else
		PrintHandle(ach, cch, matchthis);

	return -1;
}

void FillBytesList(
	HWND hwndDlg,
	HWND hwnd,
	int numBytes,
	WORD WINAPI pGetWord(HWND, int),
	DWORD WINAPI pGetLong(HWND, int),
	ULONG_PTR WINAPI pGetLongPtr(HWND, int)
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
			_stprintf_s(ach, ARRAYSIZE(ach), _T("+%-8d Unavailable (0x") szHexFmt _T(")"), i, dwLastError);

		i += chunkBytes;
		numBytes -= chunkBytes;

		LRESULT index = SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_ADDSTRING, 0, (LPARAM)ach);
		SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_SETITEMDATA, index, dwLastError == ERROR_SUCCESS ? lp : dwLastError);
	}

	SendDlgItemMessage(hwndDlg, IDC_BYTESLIST, CB_SETCURSEL, 0, 0);
}

//
//  Set the class information on the Class Tab, for the specified window.
//  This function assumes that spy_WndClassEx is completely populated.
//
void SetClassInfo(HWND hwnd)
{
	TCHAR ach[256];

	int i, numstyles, classbytes;
	HWND hwndDlg = WinSpyTab[CLASS_TAB].hwnd;
	UINT_PTR handle;

	*ach = 0;

	if (hwnd)
	{
		GetClassName(hwnd, ach, ARRAYSIZE(ach));

		// be nice and give the proper name for the following class names
		//
		VerboseClassName(ach, ARRAYSIZE(ach), (WORD)GetClassLong(hwnd, GCW_ATOM));
	}

	SetDlgItemText(hwndDlg, IDC_CLASSNAME, ach);

	//class style
	if (hwnd)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, spy_WndClassEx.style);
	}
	SetDlgItemText(hwndDlg, IDC_STYLE, ach);

	//atom
	if (hwnd)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), _T("%04X"), GetClassLong(hwnd, GCW_ATOM));
	}
	SetDlgItemText(hwndDlg, IDC_ATOM, ach);

	//extra class bytes
	if (hwnd)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), _T("%d"), spy_WndClassEx.cbClsExtra);
	}
	SetDlgItemText(hwndDlg, IDC_CLASSBYTES, ach);

	//extra window bytes
	if (hwnd)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), _T("%d"), spy_WndClassEx.cbWndExtra);
	}
	SetDlgItemText(hwndDlg, IDC_WINDOWBYTES, ach);

	//menu (not implemented)
	if (hwnd)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, (void*)GetClassLongPtr(hwnd, GCLP_MENUNAME));
	}
	SetDlgItemText(hwndDlg, IDC_MENUHANDLE, _T("(None)"));

	//cursor handle
	if (hwnd)
	{
		handle = GetClassLongPtr(hwnd, GCLP_HCURSOR);
		FormatHandle(ach, ARRAYSIZE(ach), CursorLookup, NUM_CURSOR_LOOKUP, handle);
	}
	SetDlgItemText(hwndDlg, IDC_CURSORHANDLE, ach);

	//icon handle
	if (hwnd)
	{
		handle = GetClassLongPtr(hwnd, GCLP_HICON);
		FormatHandle(ach, ARRAYSIZE(ach), IconLookup, NUM_ICON_LOOKUP, handle);
	}
	SetDlgItemText(hwndDlg, IDC_ICONHANDLE, ach);

	//background brush handle
	if (hwnd)
	{
		handle = GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND);

		//see hbrBackground description at https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577.aspx
		//first of all, search by COLOR_xxx value
		if (!(handle > 0 && handle <= MAXUINT) || (-1 == FormatConst(ach, ARRAYSIZE(ach), BrushLookup, NUM_BRUSH_STYLES, (UINT)handle - 1)))
		{
			//now search by handle value
			i = FormatHandle(ach, ARRAYSIZE(ach), BrushLookup2, NUM_BRUSH2_LOOKUP, handle);
			if (i != -1)
			{
				int len = PrintHandle(ach, ARRAYSIZE(ach), (UINT_PTR)BrushLookup2[i].handle);
				_stprintf_s(ach + len, ARRAYSIZE(ach) - len, _T("  (%s)"), BrushLookup2[i].szName);
			}
		}
	}

	//set the brush handle text
	SetDlgItemText(hwndDlg, IDC_BKGNDBRUSH, ach);

	//window procedure
	if (hwnd)
	{
		if (spy_WndProc == 0)
		{
			_tcscpy_s(ach, ARRAYSIZE(ach), _T("N/A"));
		}
		else
		{
			_stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, spy_WndProc);
			if (spy_WndProc != spy_WndClassEx.lpfnWndProc)
				_tcscat_s(ach, ARRAYSIZE(ach), _T(" (Subclassed)"));
		}
	}

	SetDlgItemText(hwndDlg, IDC_WNDPROC, ach);

	SetDlgItemText(WinSpyTab[GENERAL_TAB].hwnd, IDC_WINDOWPROC2, ach);

	//class window procedure
	if (hwnd)
	{
		if (spy_WndClassEx.lpfnWndProc == 0)
			_tcscpy_s(ach, ARRAYSIZE(ach), _T("N/A"));
		else
			_stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, spy_WndClassEx.lpfnWndProc);
	}

	SetDlgItemText(hwndDlg, IDC_CLASSPROC, ach);


	//instance handle
	if (hwnd)
	{
		_stprintf_s(ach, ARRAYSIZE(ach), szPtrFmt, spy_WndClassEx.hInstance);
	}
	SetDlgItemText(hwndDlg, IDC_INSTANCEHANDLE, ach);

	//
	// fill the combo box with the class styles
	//
	numstyles = 0;
	SendDlgItemMessage(hwndDlg, IDC_STYLELIST, CB_RESETCONTENT, 0, 0);
	if (hwnd)
	{
		for (i = 0; i < NUM_CLASS_STYLES; i++)
		{
			if (spy_WndClassEx.style & ClassLookup[i].value)
			{
				SendDlgItemMessage(hwndDlg, IDC_STYLELIST, CB_ADDSTRING, 0,
					(LPARAM)ClassLookup[i].szName);

				numstyles++;
			}
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

