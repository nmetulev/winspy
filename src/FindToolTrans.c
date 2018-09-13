//
//  FindToolTrans.c
//
//  www.catch22.net
//
//  Copyright (C) 2012 James Brown
//  Please refer to the file LICENCE.TXT for copying permission
//

#include "WinSpy.h"

#include "Utils.h"
#include "resource.h"


#define WC_TRANSWINDOW  TEXT("TransparentWindow")

HBITMAP MakeDockPanelBitmap(SIZE outputSize)
{
    static HBITMAP hbmBox;

    if (hbmBox == 0)
    {
        hbmBox = LoadPNGImage(IDB_SELBOX, NULL);
    }

    RECT edges = { 2, 2, 2, 2 };

    return ExpandNineGridImage(outputSize, hbmBox, edges);
}


//
// Very simple window-procedure for the transparent window.
//
LRESULT CALLBACK TransparentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// Creates a transparent overlay window of the same size and position of
// an existing window.
//
HWND CreateOverlayWindow(HWND hwndToCover)
{
    HWND hwnd;
    HBITMAP hbmp;
    RECT rc;
    SIZE size;

    // Initialize window class on first use.
    static BOOL fInitializedWindowClass = FALSE;

    if (!fInitializedWindowClass)
    {
        WNDCLASSEX wc = { sizeof(wc) };

        wc.style = 0;
        wc.lpszClassName = WC_TRANSWINDOW;
        wc.lpfnWndProc = TransparentWndProc;

        RegisterClassEx(&wc);

        fInitializedWindowClass = TRUE;
    }

    GetWindowRect(hwndToCover, &rc);

    hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        WC_TRANSWINDOW,
        0,
        WS_POPUP,
        rc.left, rc.top,
        GetRectWidth(&rc),
        GetRectHeight(&rc),
        0, 0, 0,
        NULL);

    size.cx = GetRectWidth(&rc);
    size.cy = GetRectHeight(&rc);
    hbmp = MakeDockPanelBitmap(size);

    UpdateLayeredWindowContent(hwnd, rc, hbmp, 220);

    DeleteObject(hbmp);

    SetWindowPos(
        hwnd,
        HWND_TOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    return hwnd;
}

