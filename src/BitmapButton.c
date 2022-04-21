//
//  BitmapButton.c
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  void MakeBitmapButton(HWND hwnd, UINT uIconId)
//
//  Converts the specified button into an owner-drawn button
//  (supports a small icon + text to the right)
//
//   hwnd    - handle to button
//   uIconId - icon resource ID (loaded from THIS module)
//
//
//  BOOL DrawBitmapButton(DRAWITEMSTRUCT *dis)
//
//  You must call this when the parent (dialog?) window receives a
//  WM_DRAWITEM for the button.
//

#include "WinSpy.h"

#include <uxtheme.h>
#include <vssym32.h> //<tmschema.h>
#include "BitmapButton.h"

BOOL    g_fThemeApiAvailable = FALSE;

HTHEME _OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
    if (g_fThemeApiAvailable)
        return OpenThemeData(hwnd, pszClassList);
    else
        return NULL;
}

HRESULT _CloseThemeData(HTHEME hTheme)
{
    if (g_fThemeApiAvailable)
        return CloseThemeData(hTheme);
    else
        return E_FAIL;
}

#ifndef ODS_NOFOCUSRECT
#define ODS_NOFOCUSRECT     0x0200
#endif

#ifndef DT_HIDEPREFIX
#define DT_HIDEPREFIX       0x100000
#endif


//
//  Subclass procedure for an owner-drawn button.
//  All this does is to re-enable double-click behavior for
//  an owner-drawn button.
//
static LRESULT CALLBACK BBProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC oldproc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    static BOOL mouseOver;
    POINT pt;
    RECT  rect;

    switch (msg)
    {
    case WM_LBUTTONDBLCLK:
        msg = WM_LBUTTONDOWN;
        break;

    case WM_MOUSEMOVE:

        if (!mouseOver)
        {
            SetTimer(hwnd, 0, 15, 0);
            mouseOver = FALSE;
        }
        break;

    case WM_TIMER:

        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);
        GetClientRect(hwnd, &rect);

        if (PtInRect(&rect, pt))
        {
            if (!mouseOver)
            {
                mouseOver = TRUE;
                InvalidateRect(hwnd, 0, 0);
            }
        }
        else
        {
            mouseOver = FALSE;
            KillTimer(hwnd, 0);
            InvalidateRect(hwnd, 0, 0);
        }

        return 0;

        // Under Win2000 / XP, Windows sends a strange message
        // to dialog controls, whenever the ALT key is pressed
        // for the first time (i.e. to show focus rect / & prefixes etc).
        // msg = 0x0128, wParam = 0x00030003, lParam = 0
    case 0x0128:
        InvalidateRect(hwnd, 0, 0);
        break;
    }

    return CallWindowProc(oldproc, hwnd, msg, wParam, lParam);
}

//BOOL DrawThemedBitmapButton(DRAWITEMSTRUCT *dis)
/*BOOL DrawBitmapButton0(DRAWITEMSTRUCT *dis)
{
    //HTHEME hTheme = GetWindowTheme(dis->hwndItem, "Button");
    HTHEME hTheme = _OpenThemeData(dis->hwndItem, L"Button");
    DWORD state;

    if(dis->itemState & ODA_FOCUS)
        ;

    if(dis->itemState & ODS_SELECTED)
        state = PBS_PRESSED;
    else if(dis->itemState & ODS_HOTLIGHT)
        state = PBS_HOT;
    else
        state = PBS_NORMAL;

    DrawThemeBackground(hTheme, dis->hDC, BP_PUSHBUTTON, state, &dis->rcItem, 0);

    _CloseThemeData(hTheme);

    return TRUE;
}*/

//
//  Call this function whenever you get a WM_DRAWITEM in the parent dialog.
//
BOOL DrawBitmapButton(DRAWITEMSTRUCT *dis)
{
    RECT rect;          // Drawing rectangle
    POINT pt;

    int ix, iy;         // Icon offset
    int bx;             // border sizes
    int cxIconBorder;
    int sxIcon, syIcon; // Icon size
    int xoff, yoff;     //

    WCHAR szText[100];
    size_t nTextLen;

    HICON hIcon;
    DWORD dwStyle = GetWindowLong(dis->hwndItem, GWL_STYLE);

    DWORD dwDTflags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
    BOOL  fRightAlign;

    // XP/Vista theme support
    DWORD dwThemeFlags;
    HTHEME hTheme;
    //BOOL   fDrawThemed = g_fThemeApiAvailable;

    if (dis->itemState & ODS_NOFOCUSRECT)
        dwDTflags |= DT_HIDEPREFIX;

    fRightAlign = (dwStyle & BS_RIGHT) ? TRUE : FALSE;

    // do the theme thing
    hTheme = _OpenThemeData(dis->hwndItem, L"Button");

    switch (dis->itemAction)
    {
        // We need to redraw the whole button, no
        // matter what DRAWITEM event we receive.
    case ODA_FOCUS:
    case ODA_SELECT:
    case ODA_DRAWENTIRE:

        // Retrieve button text
        GetWindowText(dis->hwndItem, szText, ARRAYSIZE(szText));

        nTextLen = wcslen(szText);

        // Retrieve button icon
        hIcon = (HICON)SendMessage(dis->hwndItem, BM_GETIMAGE, IMAGE_ICON, 0);

        // Find icon dimensions
        sxIcon = syIcon = DPIScale(dis->hwndItem, 16);

        CopyRect(&rect, &dis->rcItem);
        GetCursorPos(&pt);
        ScreenToClient(dis->hwndItem, &pt);

        if (PtInRect(&rect, pt))
            dis->itemState |= ODS_HOTLIGHT;

        // border dimensions
        bx = DPIScale(dis->hwndItem, 2);
        cxIconBorder = DPIScale(dis->hwndItem, 3);

        // icon offsets
        if (nTextLen == 0)
        {
            // center the image if no text
            ix = (GetRectWidth(&rect) - sxIcon) / 2;
        }
        else
        {
            if (fRightAlign)
                ix = rect.right - bx - cxIconBorder - sxIcon;
            else
                ix = rect.left + bx + cxIconBorder;
        }

        // center image vertically
        iy = (GetRectHeight(&rect) - syIcon) / 2;

        InflateRect(&rect, -5, -5);

        // Draw a single-line black border around the button
        if (hTheme == NULL && (dis->itemState & (ODS_FOCUS | ODS_DEFAULT)))
        {
            FrameRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            InflateRect(&dis->rcItem, -1, -1);
        }

        if (dis->itemState & ODS_FOCUS)
            dwThemeFlags = PBS_DEFAULTED;
        if (dis->itemState & ODS_DISABLED)
            dwThemeFlags = PBS_DISABLED;
        else if (dis->itemState & ODS_SELECTED)
            dwThemeFlags = PBS_PRESSED;
        else if (dis->itemState & ODS_HOTLIGHT)
            dwThemeFlags = PBS_HOT;
        else if (dis->itemState & ODS_DEFAULT)
            dwThemeFlags = PBS_DEFAULTED;
        else
            dwThemeFlags = PBS_NORMAL;

        // Button is DOWN
        if (dis->itemState & ODS_SELECTED)
        {
            // Draw a button
            if (hTheme != NULL)
                DrawThemeBackground(hTheme, dis->hDC, BP_PUSHBUTTON, dwThemeFlags, &dis->rcItem, 0);
            else
                DrawFrameControl(dis->hDC, &dis->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED | DFCS_FLAT);


            // Offset contents to make it look "pressed"
            if (hTheme == NULL)
            {
                OffsetRect(&rect, 1, 1);
                xoff = yoff = 1;
            }
            else
                xoff = yoff = 0;
        }
        // Button is UP
        else
        {
            //
            if (hTheme != NULL)
                DrawThemeBackground(hTheme, dis->hDC, BP_PUSHBUTTON, dwThemeFlags, &dis->rcItem, 0);
            else
                DrawFrameControl(dis->hDC, &dis->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH);

            xoff = yoff = 0;
        }

        // Draw the icon
        DrawIconEx(dis->hDC, ix + xoff, iy + yoff, hIcon, sxIcon, syIcon, 0, 0, DI_NORMAL);

        // Adjust position of window text
        if (fRightAlign)
        {
            rect.left += bx + cxIconBorder;
            rect.right -= sxIcon + bx + cxIconBorder;
        }
        else
        {
            rect.right -= bx + cxIconBorder;
            rect.left += sxIcon + bx + cxIconBorder;
        }

        // Draw the text
        OffsetRect(&rect, 0, -1);
        SetBkMode(dis->hDC, TRANSPARENT);
        DrawText(dis->hDC, szText, -1, &rect, dwDTflags);
        OffsetRect(&rect, 0, 1);

        // Draw the focus rectangle
        if (dis->itemState & ODS_FOCUS)
        {
            if (!(dis->itemState & ODS_NOFOCUSRECT))
            {
                // Get a "fresh" copy of the button rectangle
                CopyRect(&rect, &dis->rcItem);

                if (nTextLen > 0)
                {
                    if (fRightAlign)
                        rect.right -= sxIcon + bx;
                    else
                        rect.left += sxIcon + bx + 2;
                }

                int cx = 2 + DPIScale(dis->hwndItem, 1);

                InflateRect(&rect, -cx, -cx);

                DrawFocusRect(dis->hDC, &rect);
            }
        }

        break;
    }

    _CloseThemeData(hTheme);
    return TRUE;
}

//
//  Convert the specified button into an owner-drawn button.
//  The button does NOT need owner-draw or icon styles set
//  in the resource editor - this function sets these
//  styles automatically
//
void MakeBitmapButton(HWND hwnd, UINT uIconId)
{
    WNDPROC oldproc;
    DWORD   dwStyle;
    int     cxIcon = DPIScale(hwnd, 16);

    HICON hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(uIconId), IMAGE_ICON, cxIcon, cxIcon, 0);

    // Add on BS_ICON and BS_OWNERDRAW styles
    dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, dwStyle | BS_ICON | BS_OWNERDRAW);

    // Assign icon to the button
    SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

    // Subclass (to reenable double-clicks)
    oldproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)BBProc);

    // Store old procedure
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)oldproc);

    if (g_fThemeApiAvailable)
        SetWindowTheme(hwnd, L"explorer", NULL);
}

//
//  Just a helper function really
//
void MakeDlgBitmapButton(HWND hwndDlg, UINT uCtrlId, UINT uIconId)
{
    if (GetModuleHandle(L"uxtheme.dll"))
        g_fThemeApiAvailable = TRUE;
    else
        g_fThemeApiAvailable = FALSE;

    MakeBitmapButton(GetDlgItem(hwndDlg, uCtrlId), uIconId);
}
