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

HWND CreateOverlayWindow(HWND hwndToCover);

static LONG    g_lRefCount = 0;

//
// Handle to the two dragger bitmaps
//
static HBITMAP hBitmapDrag1, hBitmapDrag2;
static HCURSOR hCursor;

// Old window procedure...?
static WNDPROC oldstaticproc;


//
// These globals are valid while a finder drag operation is active.
//

static BOOL    g_fDragging;
static HWND    g_hwndFinder;    // The active finder tool window
static HWND    g_hwndOverlay;   // The overlay/highlight window
static HWND    g_hwndCurrent;   // The currently selected window
static POINT   g_ptLast;        // Position of last mouse move
static HCURSOR g_hOldCursor;
static HHOOK   g_draghook;
static BOOL    g_fAltDown;      // Is the alt key pressed?


void LoadFinderResources()
{
    hBitmapDrag1 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_DRAGTOOL1));
    hBitmapDrag2 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_DRAGTOOL2));

    hCursor = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_CURSOR1));
}

void FreeFinderResources()
{
    DeleteObject(hBitmapDrag1);
    DeleteObject(hBitmapDrag2);

    DestroyCursor(hCursor);
}

void FindTool_ShowOverlay()
{
    g_hwndOverlay = CreateOverlayWindow(g_hwndCurrent);
}

void FindTool_RemoveOverlay()
{
    DestroyWindow(g_hwndOverlay);
    g_hwndOverlay = NULL;
}

UINT FindTool_FireNotify(UINT uCode, HWND hwnd)
{
    WNDFINDPROC wfp = (WNDFINDPROC)GetWindowLongPtr(g_hwndFinder, GWLP_USERDATA);
    UINT result = 0;

    // Hide the selection overlay during the callout.
    if (g_hwndCurrent)
    {
        FindTool_RemoveOverlay();
    }

    if (wfp != 0)
    {
        result = wfp(g_hwndFinder, uCode, hwnd);
    }

    // On selection changed event, the parameter becomes the current selection.
    // This means that in the WFN_SELCHANGED case we remove the overlay from
    // the old window and show it one the new one.
    if (uCode == WFN_SELCHANGED)
    {
        g_hwndCurrent = hwnd;
    }

    // Restore the selection overlay.
    // Note that in the WFN_BEGIN, WFN_END, and WFN_CANCELLED cases the current
    // window hasn't been set or has already been cleared.
    if (g_hwndCurrent)
    {
        FindTool_ShowOverlay();
    }

    return result;
}

void FindTool_UpdateSelectionFromPoint(POINT pt)
{
    HWND hwndPoint;

    ClientToScreen(g_hwndFinder, (POINT *)&pt);

    hwndPoint = WindowFromPointEx(pt, g_fAltDown, g_opts.fShowHidden);

    if (hwndPoint && (hwndPoint != g_hwndCurrent))
    {
        FindTool_FireNotify(WFN_SELCHANGED, hwndPoint);
    }
}

// Keyboard hook for the Finder Tool.
// This hook just monitors the ESCAPE key
static LRESULT CALLBACK draghookproc(int code, WPARAM wParam, LPARAM lParam)
{
    BOOL newStateReleased = (ULONG)lParam & (1 << 31);
    BOOL previousStateDown = (ULONG)lParam & (1 << 30);
    static int count;

    if (code != HC_ACTION)
        return CallNextHookEx(g_draghook, code, wParam, lParam);

    switch (wParam)
    {
    case VK_ESCAPE:

        if (!newStateReleased)
        {
            //don't let the current window procedure process a VK_ESCAPE,
            //because we want it to cancel the mouse capture
            PostMessage(g_hwndFinder, WM_CANCELMODE, 0, 0);
            return -1;
        }

        break;

    case VK_SHIFT:

        if (newStateReleased)
        {
            FindTool_FireNotify(WFN_SHIFT_UP, 0);
        }
        else if (!previousStateDown)
        {
            FindTool_FireNotify(WFN_SHIFT_DOWN, 0);
        }

        return -1;

    case VK_CONTROL:

        if (newStateReleased)
        {
            FindTool_FireNotify(WFN_CTRL_UP, 0);
        }
        else if (!previousStateDown)
        {
            FindTool_FireNotify(WFN_CTRL_DOWN, 0);
        }

        return -1;

    case VK_MENU: // Alt Key

        g_fAltDown = !newStateReleased;

        // Re-evaluate the window under the cursor.
        FindTool_UpdateSelectionFromPoint(g_ptLast);

        return -1;
    }

    // Test to see if a key is pressed for first time
    if (!(newStateReleased || previousStateDown))
    {
        // Find ASCII character
        UINT ch = MapVirtualKey((UINT)wParam, 2);

        if (ch == 'c' || ch == 'C')
        {
            FindTool_RemoveOverlay();
            CaptureWindow(GetParent(g_hwndFinder), g_hwndCurrent);
            FindTool_ShowOverlay();
            return -1;
        }
    }

    return CallNextHookEx(g_draghook, code, wParam, lParam);
}

void FindTool_BeginDrag(HWND hwnd, LPARAM lParam)
{
    g_ptLast.x = (short)LOWORD(lParam);
    g_ptLast.y = (short)HIWORD(lParam);

    g_hwndFinder = hwnd;

    // Ask the callback function if we want to proceed
    if (FindTool_FireNotify(WFN_BEGIN, 0) != -1)
    {
        g_fDragging = TRUE;
        g_fAltDown = ((GetKeyState(VK_MENU) & 0x8000) != 0);

        SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmapDrag2);

        SetCapture(hwnd);
        g_hOldCursor = SetCursor(hCursor);

        // Install keyboard hook to trap ESCAPE key
        // I don't see how our window can get the mouse capture
        // without also getting the keyboard focus,
        // but attempting to install a desktop-wide hook will definitely fail,
        // so let's install our keyboard hook for this thread only - at least that works
        g_draghook = SetWindowsHookEx(WH_KEYBOARD, draghookproc, g_hInst, GetCurrentThreadId());

        // Select initial window.
        g_hwndCurrent = NULL;
        FindTool_UpdateSelectionFromPoint(g_ptLast);
    }
    else
    {
        g_hwndFinder = NULL;
    }
}

LRESULT FindTool_EndDrag(UINT uCode)
{
    FindTool_RemoveOverlay();
    ReleaseCapture();
    SetCursor(g_hOldCursor);

    // Remove keyboard hook. This is done even if the user presses ESC
    UnhookWindowsHookEx(g_draghook);

    g_fDragging = FALSE;
    SendMessage(g_hwndFinder, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmapDrag1);

    // Final notification.
    HWND hwndForNotify = (uCode == WFN_END) ? g_hwndCurrent : NULL;

    g_hwndCurrent = NULL;

    FindTool_FireNotify(uCode, hwndForNotify);

    return 0;
}

LRESULT CALLBACK StaticProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:

        FindTool_BeginDrag(hwnd, lParam);
        return 0;

    case WM_MOUSEMOVE:

        if (g_fDragging)
        {
            POINT pt;

            pt.x = (short)LOWORD(lParam);
            pt.y = (short)HIWORD(lParam);

            if (!(g_ptLast.x == pt.x && g_ptLast.y == pt.y))
            {
                g_ptLast = pt;
                FindTool_UpdateSelectionFromPoint(pt);
            }
        }
        return 0;

    case WM_LBUTTONUP:

        // Mouse has been released, so end the find-tool
        if (g_fDragging)
        {
            FindTool_EndDrag(WFN_END);
        }
        return 0;

        // Sent from the keyboard hook
    case WM_CANCELMODE:

        // User has pressed ESCAPE, so cancel the find-tool
        if (g_fDragging)
        {
            FindTool_EndDrag(WFN_CANCELLED);
        }
        return 0;

    case WM_NCDESTROY:

        // When the last finder tool has been destroyed, free
        // up all the resources
        if (InterlockedDecrement(&g_lRefCount) == 0)
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
    if (InterlockedIncrement(&g_lRefCount) == 1)
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

