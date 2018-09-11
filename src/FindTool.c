//
//  WinSpy Finder Tool.
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  This is a standalone file which implements
//  a "Finder Tool" similar to that used in Spy++
//
//  There are two functions you must use:
//
//  1. BOOL MakeFinderTool(HWND hwnd, WNDFINDPROC wfp)
//
//     hwnd  - handle to a STATIC control to base the tool around.
//             MakeFinderTool converts this control to the correct
//             style, adds the bitmaps and mouse support etc.
//
//     wfn   - Event callback function. Must not be zero.
//
//     Return values:
//             TRUE for success, FALSE for failure
//
//
//  2. UINT CALLBACK WndFindProc(HWND hwndTool, UINT uCode, HWND hwnd)
//
//     This is a callback function that you supply when using
//     MakeFinderTool. This callback can be executed for a number
//     different events - described by uCode.
//
//     hwndTool - handle to the finder tool
//
//     hwnd  - handle to the window which has been found.
//
//     uCode - describes the event. Can be one of the following values.
//
//             WFN_BEGIN        : tool is about to become active.
//             WFN_SELCHANGING  : sent when tool moves from window-window.
//             WFN_SELCHANGED   : sent when final window is selected.
//             WFN_CANCELLED    : Tool cancelled. hwnd is not valid (0)
//
//     Return values:
//             Return value is only checked for WFN_BEGIN. Return 0 (zero)
//             to continue, -1 to prevent tool from being used. Otherwise,
//             return 0 (zero) for all other messages
//

#include "WinSpy.h"

#include "FindTool.h"
#include "WindowFromPointEx.h"
#include "resource.h"
#include "CaptureWindow.h"

#define INVERT_BORDER 3

HWND ShowTransWindow(HWND);
void ShowSel(HWND);
void HideSel(HWND);

static BOOL fTransSel = TRUE;
static HWND hwndTransPanel = 0;

static LONG    lRefCount = 0;

static HCURSOR hOldCursor;
static HHOOK   draghook = 0;
static HWND    draghookhwnd = 0;

//
//  Handle to the two dragger bitmaps
//
static HBITMAP hBitmapDrag1, hBitmapDrag2;
static HCURSOR hCursor;

//is the finder-tool being dragged??
static BOOL fDragging = FALSE;

// Old window procedure...?
static WNDPROC oldstaticproc;


static HWND hwndCurrent;

//
//  Invert the specified window's border
//
void InvertWindow(HWND hwnd, BOOL fUseScreenDC)
{
	RECT rect;
	RECT rect2;
	//RECT rectc;
	HDC hdc;
	int x1, y1;

	int border = INVERT_BORDER;

	if (hwnd == 0)
		return;

	//window rectangle (screen coords)
	GetWindowRect(hwnd, &rect);

	////client rectangle (screen coords)
	//GetClientRect(hwnd, &rectc);
	//ClientToScreen(hwnd, (POINT *)&rectc.left);
	//ClientToScreen(hwnd, (POINT *)&rectc.right);
	////MapWindowPoints(hwnd, 0, (POINT *)&rectc, 2);

	x1 = rect.left;
	y1 = rect.top;
	OffsetRect(&rect, -x1, -y1);
	//OffsetRect(&rectc, -x1, -y1);

	if (rect.bottom - border * 2 < 0)
		border = 1;

	if (rect.right - border * 2 < 0)
		border = 1;

	if (fUseScreenDC)
		hwnd = 0;

	hdc = GetWindowDC(hwnd);

	if (hdc == 0)
		return;

	//top edge
	//border = rectc.top-rect.top;
	SetRect(&rect2, 0, 0, rect.right, border);
	if (fUseScreenDC) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	//left edge
	//border = rectc.left-rect.left;
	SetRect(&rect2, 0, border, border, rect.bottom);
	if (fUseScreenDC) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	//right edge
	//border = rect.right-rectc.right;
	SetRect(&rect2, border, rect.bottom - border, rect.right, rect.bottom);
	if (fUseScreenDC) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	//bottom edge
	//border = rect.bottom-rectc.bottom;
	SetRect(&rect2, rect.right - border, border, rect.right, rect.bottom - border);
	if (fUseScreenDC) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	ReleaseDC(hwnd, hdc);
}

void FlashWindowBorder(HWND hwnd)
{
	int i;

	for (i = 0; i < 3 * 2; i++)
	{
		InvertWindow(hwnd, TRUE);
		Sleep(100);
	}
}

void LoadFinderResources()
{
	hBitmapDrag1 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DRAGTOOL1));
	hBitmapDrag2 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DRAGTOOL2));

	hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR1));
}

void FreeFinderResources()
{
	DeleteObject(hBitmapDrag1);
	DeleteObject(hBitmapDrag2);

	DestroyCursor(hCursor);
}

WNDFINDPROC GetWndFindProc(HWND hwnd)
{
	return (WNDFINDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

UINT FireWndFindNotify(HWND hwndTool, UINT uCode, HWND hwnd)
{
	WNDFINDPROC wfp = GetWndFindProc(hwndTool);

	if (wfp != 0)
		return wfp(hwndTool, uCode, hwnd);
	else
		return 0;
}

LRESULT EndFindToolDrag(HWND hwnd)
{
	//InvertWindow(hwndCurrent, fShowHidden);
	HideSel(hwndCurrent);
	ReleaseCapture();
	SetCursor(hOldCursor);

	// Remove keyboard hook. This is done even if the user presses ESC
	UnhookWindowsHookEx(draghook);


	fDragging = FALSE;
	SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmapDrag1);

	return 0;
}

// Keyboard hook for the Finder Tool.
// This hook just monitors the ESCAPE key
static LRESULT CALLBACK draghookproc(int code, WPARAM wParam, LPARAM lParam)
{
	BOOL newStateReleased = (ULONG)lParam & (1 << 31);
	BOOL previousStateDown = (ULONG)lParam & (1 << 30);
	static int count;

	if (code < 0)
		return CallNextHookEx(draghook, code, wParam, lParam);

	if (code != HC_ACTION)
		return CallNextHookEx(draghook, code, wParam, lParam);

	switch (wParam)
	{
	case VK_ESCAPE:

		if (!newStateReleased)
		{
			//don't let the current window procedure process a VK_ESCAPE,
			//because we want it to cancel the mouse capture
			PostMessage(draghookhwnd, WM_CANCELMODE, 0, 0);
			return -1;
		}

		break;

	case VK_SHIFT:

		if (newStateReleased)
		{
			//InvertWindow(hwndCurrent, fShowHidden);
			HideSel(hwndCurrent);
			FireWndFindNotify(draghookhwnd, WFN_SHIFT_UP, 0);
			//InvertWindow(hwndCurrent, fShowHidden);
			ShowSel(hwndCurrent);
		}
		else
		{
			if (!previousStateDown)
			{
				//InvertWindow(hwndCurrent, fShowHidden);
				HideSel(hwndCurrent);
				FireWndFindNotify(draghookhwnd, WFN_SHIFT_DOWN, 0);
				//InvertWindow(hwndCurrent, fShowHidden);
				ShowSel(hwndCurrent);
			}
		}

		return -1;

	case VK_CONTROL:

		if (newStateReleased)
		{
			//InvertWindow(hwndCurrent, fShowHidden);
			HideSel(hwndCurrent);
			FireWndFindNotify(draghookhwnd, WFN_CTRL_UP, 0);
			//InvertWindow(hwndCurrent, fShowHidden);
			ShowSel(hwndCurrent);
		}
		else
		{
			if (!previousStateDown)
			{
				//InvertWindow(hwndCurrent, fShowHidden);
				HideSel(hwndCurrent);
				FireWndFindNotify(draghookhwnd, WFN_CTRL_DOWN, 0);
				//InvertWindow(hwndCurrent, fShowHidden);
				ShowSel(hwndCurrent);
			}
		}

		return -1;
	}

	// Test to see if a key is pressed for first time
	if (!(newStateReleased || previousStateDown))
	{
		// Find ASCII character
		UINT ch = MapVirtualKey((UINT)wParam, 2);

		if (ch == _T('c') || ch == _T('C'))
		{
			//InvertWindow(hwndCurrent, fShowHidden);
			HideSel(hwndCurrent);
			CaptureWindow(GetParent(draghookhwnd), hwndCurrent);
			//InvertWindow(hwndCurrent, fShowHidden);
			ShowSel(hwndCurrent);
			return -1;
		}
	}

	return CallNextHookEx(draghook, code, wParam, lParam);
}

void ShowSel(HWND hwnd)
{
	if (fTransSel)
	{
		hwndTransPanel = ShowTransWindow(hwnd);

		if (hwndTransPanel == 0)
		{
			fTransSel = FALSE;
			InvertWindow(hwnd, fShowHidden);
		}
	}
	else
	{
		InvertWindow(hwnd, fShowHidden);
	}
}

void HideSel(HWND hwnd)
{
	if (fTransSel)
	{
		DestroyWindow(hwndTransPanel);
		hwndTransPanel = 0;
	}
	else
	{
		InvertWindow(hwnd, fShowHidden);
	}
}


LRESULT CALLBACK StaticProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;

	static POINT ptLast;

	switch (msg)
	{
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:

		ptLast.x = (short)LOWORD(lParam);
		ptLast.y = (short)HIWORD(lParam);

		// Ask the callback function if we want to proceed
		if (FireWndFindNotify(hwnd, WFN_BEGIN, 0) == -1)
		{
			return 0;
		}

		fDragging = TRUE;

		SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmapDrag2);

		hwndCurrent = hwnd;

		ShowSel(hwndCurrent);

		SetCapture(hwnd);
		hOldCursor = SetCursor(hCursor);

		// Install keyboard hook to trap ESCAPE key
		// I don't see how our window can get the mouse capture
		// without also getting the keyboard focus,
		// but attempting to install a desktop-wide hook will definitely fail,
		// so let's install our keyboard hook for this thread only - at least that works
		draghookhwnd = hwnd;
		draghook = SetWindowsHookEx(WH_KEYBOARD, draghookproc, hInst, GetCurrentThreadId());

		// Current window has changed
		FireWndFindNotify(hwnd, WFN_SELCHANGED, hwndCurrent);

		return 0;

	case WM_MOUSEMOVE:

		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);

		if (fDragging && !(ptLast.x == pt.x && ptLast.y == pt.y))
		{
			//MoveFindTool(hwnd, wParam, lParam);

			HWND hWndPoint;

			ptLast = pt;
			ClientToScreen(hwnd, (POINT *)&pt);

			hWndPoint = WindowFromPointEx(pt, fShowHidden);

			if (hWndPoint == 0)
				return 0;

			if (hWndPoint != hwndCurrent)
			{
				HideSel(hwndCurrent);
				//InvertWindow(hwndCurrent, fShowHidden);

				FireWndFindNotify(hwnd, WFN_SELCHANGED, hWndPoint);
				//InvertWindow(hWndPoint, fShowHidden);
				ShowSel(hWndPoint);

				hwndCurrent = hWndPoint;
			}
		}
		return 0;

	case WM_LBUTTONUP:

		// Mouse has been released, so end the find-tool
		if (fDragging)
		{
			fDragging = FALSE;

			EndFindToolDrag(hwnd);
			FireWndFindNotify(hwnd, WFN_END, hwndCurrent);
		}

		return 0;

		// Sent from the keyboard hook
	case WM_CANCELMODE:

		// User has pressed ESCAPE, so cancel the find-tool
		if (fDragging)
		{
			fDragging = FALSE;

			EndFindToolDrag(hwnd);
			FireWndFindNotify(hwnd, WFN_CANCELLED, 0);
		}

		return 0;

	case WM_NCDESTROY:

		// When the last finder tool has been destroyed, free
		// up all the resources
		if (InterlockedDecrement(&lRefCount) == 0)
		{
			FreeFinderResources();
		}

		break;
	}

	return CallWindowProc(oldstaticproc, hwnd, msg, wParam, lParam);
}


BOOL MakeFinderTool(HWND hwnd, WNDFINDPROC wfp)
{
	DWORD dwStyle;

	// If this is the first finder tool, then load
	// the bitmap and mouse-cursor resources
	if (InterlockedIncrement(&lRefCount) == 1)
	{
		LoadFinderResources();
	}

	// Apply styles to make this a picture control
	dwStyle = GetWindowLong(hwnd, GWL_STYLE);

	// Turn OFF styles we don't want
	dwStyle &= ~(SS_RIGHT | SS_CENTER | SS_CENTERIMAGE);
	dwStyle &= ~(SS_ICON | SS_SIMPLE | SS_LEFTNOWORDWRAP);

	// Turn ON styles we must have
	dwStyle |= SS_NOTIFY;
	dwStyle |= SS_BITMAP;

	// Now apply them.
	SetWindowLong(hwnd, GWL_STYLE, dwStyle);

	// Set the default bitmap
	SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmapDrag1);

	// Set the callback for this control
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wfp);

	// Subclass the static control
	oldstaticproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)StaticProc);

	return TRUE;
}

