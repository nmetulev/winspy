//
//  DockTransPanel.c
//
//  www.catch22.net
//
//  Copyright (C) 2012 James Brown
//  Please refer to the file LICENCE.TXT for copying permission
//

#include "WinSpy.h"

#include "resource.h"


HBITMAP LoadPNGImage(UINT id, void **bits);

#define WC_TRANSWINDOW  TEXT("TransWindow")

HBITMAP ExpandNineGridImage(SIZE outputSize, HBITMAP hbmSrc, RECT edges)
{
    HDC     hdcScreen, hdcDst, hdcSrc;
    HBITMAP hbmDst;
    void*   pBits;
    HANDLE  hOldSrc, hOldDst;
    BITMAP  bmSrc;
    
    // Create a 32bpp DIB of the desired size, this is the output bitmap.
    BITMAPINFOHEADER bih = { sizeof(bih) };

    bih.biWidth       = outputSize.cx;
    bih.biHeight      = outputSize.cy;
    bih.biPlanes      = 1;
    bih.biBitCount    = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage   = 0;

    hdcScreen = GetDC(0);
    hbmDst = CreateDIBSection(hdcScreen, (BITMAPINFO *)&bih, DIB_RGB_COLORS, &pBits, 0, 0);
    
    // Determine size of the source image.
    GetObject(hbmSrc, sizeof(bmSrc), &bmSrc);
    
    // Prep DCs
    hdcSrc = CreateCompatibleDC(hdcScreen);
    hOldSrc = SelectObject(hdcSrc, hbmSrc);
    
    hdcDst = CreateCompatibleDC(hdcScreen);
    hOldDst = SelectObject(hdcDst, hbmDst);

    // Sizes of the nine-grid edges
    int cxEdgeL = edges.left;
    int cxEdgeR = edges.right;
    int cyEdgeT = edges.top;
    int cyEdgeB = edges.bottom;
               
    // Precompute sizes and coordinates of the interior boxes 
    // (that is, the source and dest rects with the edges subtracted out).
    int cxDstInner = outputSize.cx - (cxEdgeL + cxEdgeR);
    int cyDstInner = outputSize.cy - (cyEdgeT + cyEdgeB);
    int cxSrcInner = bmSrc.bmWidth - (cxEdgeL + cxEdgeR);
    int cySrcInner = bmSrc.bmHeight - (cyEdgeT + cyEdgeB);
    
    int xDst1 = cxEdgeL;
    int xDst2 = outputSize.cx - cxEdgeR;
    int yDst1 = cyEdgeT;
    int yDst2 = outputSize.cy - cyEdgeB;
    
    int xSrc1 = cxEdgeL;
    int xSrc2 = bmSrc.bmWidth - cxEdgeR;
    int ySrc1 = cyEdgeT;
    int ySrc2 = bmSrc.bmHeight - cyEdgeB;
    
    // Upper-left corner
    BitBlt(
        hdcDst, 0, 0, cxEdgeL, cyEdgeT, 
        hdcSrc, 0, 0, 
        SRCCOPY);
    
    // Upper-right corner
    BitBlt(
        hdcDst, xDst2, 0, cxEdgeR, cyEdgeT, 
        hdcSrc, xSrc2, 0, 
        SRCCOPY);
    
    // Lower-left corner
    BitBlt(
        hdcDst, 0, yDst2, cxEdgeL, cyEdgeB, 
        hdcSrc, 0, ySrc2, 
        SRCCOPY);
    
    // Lower-right corner
    BitBlt(
        hdcDst, xDst2, yDst2, cxEdgeR, cyEdgeB, 
        hdcSrc, xSrc2, ySrc2, 
        SRCCOPY);

    // Left side
    StretchBlt(
        hdcDst, 0, yDst1, cxEdgeL, cyDstInner, 
        hdcSrc, 0, ySrc1, cxEdgeL, cySrcInner, 
        SRCCOPY);
    
    // Right side
    StretchBlt(
        hdcDst, xDst2, yDst1, cxEdgeR, cyDstInner, 
        hdcSrc, xSrc2, ySrc1, cxEdgeR, cySrcInner, 
        SRCCOPY);
    
    // Top side
    StretchBlt(
        hdcDst, xDst1, 0, cxDstInner, cyEdgeT, 
        hdcSrc, xSrc1, 0, cxSrcInner, cyEdgeT,    
        SRCCOPY);
    
    // Bottom side
    StretchBlt(
        hdcDst, xDst1, yDst2, cxDstInner, cyEdgeB, 
        hdcSrc, xSrc1, ySrc2, cxSrcInner, cyEdgeB,    
        SRCCOPY);
        
    // Middle
    StretchBlt(
        hdcDst, xDst1, yDst1, cxDstInner, cyDstInner, 
        hdcSrc, xSrc1, ySrc1, cxSrcInner, cySrcInner, 
        SRCCOPY);

    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);

    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);

    ReleaseDC(0, hdcScreen);
    
    return hbmDst;
}

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

void UpdatePanelTrans(HWND hwndPanel, RECT *rect)
{
	POINT ptZero = { 0, 0 };
	COLORREF crKey = RGB(0, 0, 0);

	const BYTE SourceConstantAlpha = 220;//255;
	BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, 0, AC_SRC_ALPHA };
	blendPixelFunction.SourceConstantAlpha = SourceConstantAlpha;

	POINT pt;
	pt.x = rect->left;
	pt.y = rect->top;
	SIZE sz;
	sz.cx = GetRectWidth(rect);
	sz.cy = GetRectHeight(rect);

	HDC hdcSrc = GetDC(0);
	HDC hdcMem = CreateCompatibleDC(hdcSrc);
	HBITMAP hbm;
	HANDLE hold;

	hbm = MakeDockPanelBitmap(sz);
	hold = SelectObject(hdcMem, hbm);

	UpdateLayeredWindow(hwndPanel,
		hdcSrc,
		&pt, //pos
		&sz, //size
		hdcMem,
		&ptZero,
		crKey,
		&blendPixelFunction,
		ULW_ALPHA);

	SelectObject(hdcMem, hold);
	DeleteDC(hdcMem);
	ReleaseDC(0, hdcSrc);

    DeleteObject(hbm);
}

//
//  Very simple window-procedure for the transparent window
//  all the drawing happens via the DOCKPANEL WM_TIMER,
//  and calls to UpdateLayeredWindow with a transparent PNG graphic
//
LRESULT CALLBACK TransWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NCHITTEST:
		return HTTRANSPARENT;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

ATOM InitTrans()
{
	WNDCLASSEX wc = { sizeof(wc) };

	wc.style = 0;
	wc.lpszClassName = WC_TRANSWINDOW;
	wc.lpfnWndProc = TransWndProc;

	return RegisterClassEx(&wc);
}


HWND ShowTransWindow(HWND hwnd)//, RECT *rect)
{
	HWND hwndTransPanel;
	RECT r, rect;

	__try
	{
		GetWindowRect(hwnd, &r);
		rect = r;

		InitTrans();

		hwndTransPanel = CreateWindowEx(
			WS_EX_TOOLWINDOW | WS_EX_LAYERED,
			WC_TRANSWINDOW,
			0,
			WS_POPUP,
			r.left, r.top,
			r.right - r.left,
			r.bottom - r.top,
			0, 0, 0, &rect);

		UpdatePanelTrans(hwndTransPanel, &r);

		SetWindowPos(hwndTransPanel, HWND_TOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		return hwndTransPanel;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}