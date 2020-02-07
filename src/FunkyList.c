//
//  WinSpy Finder Tool.
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Nice-looking owner-drawn list (used for style-lists).
//

#include "WinSpy.h"

//
//  Called from WM_MEASUREITEM
//
BOOL FunkyList_MeasureItem(HWND hwnd, MEASUREITEMSTRUCT *mis)
{
    // For a LBS_OWNERDRAWFIXED listbox, WM_MEASUREITEM is sent just once
    // from the WM_CREATE handler of the listbox window.  At that point in
    // time the owning dialog's font has not yet been set on the listbox, and
    // the default itemHeight in the MEASUREITEMSTRUCT will have been derived
    // from the default DC font (what you get on a DC that doesn't have any
    // explicitly selected font).  The default DC metrics are fixed and are
    // really only suitable when running at 100% DPI.  If running at any DPI
    // awareness level other than unaware, and at a DPI scale above 100% then
    // that default height will cause the bottom of text to be clipped.
    //
    // To gracefully handle running at high DPI we need to explicitly measure
    // the height of the text here.  We can query the font to be used from
    // the parent of the listbox (i.e. the owning dialog).

    HWND hwndDialog = GetParent(hwnd);
    HFONT hfont = (HFONT)SendMessageA(hwndDialog, WM_GETFONT, 0, 0);

    HDC hdc = GetDC(hwnd);
    HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);
    TEXTMETRIC tm;

    if (GetTextMetrics(hdc, &tm))
    {
        mis->itemHeight = tm.tmHeight;
    }

    SelectObject(hdc, hfontOld);
    ReleaseDC(hwnd, hdc);

    return TRUE;
}

//
//  Super owner-drawn list!
//
//  All we do is draw the list normally, but with a couple of minor changes:
//
//  Each list item will have its user-defined dataitem set to the definition
//  of the style it represents.
//
//  If this style's value is zero, this means that it is an implicit style, so
//  draw the whole line gray.
//
//  Also, at the end of every line, right-align the hex-values of each style
//
BOOL FunkyList_DrawItem(HWND hwnd, UINT uCtrlId, DRAWITEMSTRUCT *dis)
{
	HWND  hwndList = GetDlgItem(hwnd, uCtrlId);
	TCHAR szText[MAX_STYLE_NAME_CCH];

	COLORREF crFG = GetTextColor(dis->hDC);
	COLORREF crBG = GetBkColor(dis->hDC);

	switch (dis->itemAction)
	{
	case ODA_FOCUS:
		DrawFocusRect(dis->hDC, &dis->rcItem);
		break;

	case ODA_SELECT:
	case ODA_DRAWENTIRE:

		// get the text string to display, and the item state.
		// In general, calling LB_GETTEXT is not safe unless we are sure the text length does not exceed our buffer.
		// Therefore, we use a loose equivalent of a static_assert in the definition of the NAMEANDVALUE_ macro to make sure that our style name lengths never exceed MAX_STYLE_NAME_CCH
		static_assert(ARRAYSIZE(szText) >= MAX_STYLE_NAME_CCH, "Buffer length is smaller than the maximum possible item text length");
		SendMessage(hwndList, LB_GETTEXT, dis->itemID, (LONG_PTR)szText);
		StyleLookupEx *pStyle = (StyleLookupEx *)dis->itemData;

		if ((dis->itemState & ODS_SELECTED))
		{
			SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHT));
		}
		else
		{
			// Make the item greyed-out if the style is zero
			if (pStyle && pStyle->value == 0)
				SetTextColor(dis->hDC, GetSysColor(COLOR_3DSHADOW));
			else
				SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));

			SetBkColor(dis->hDC, GetSysColor(COLOR_WINDOW));
		}

		//draw the item text first of all. The ExtTextOut function also
		//lets us draw a rectangle under the text, so we use this facility
		//to draw the whole line at once.
		ExtTextOut(dis->hDC,
			dis->rcItem.left + 2,
			dis->rcItem.top + 0,
			ETO_OPAQUE, &dis->rcItem, szText, (UINT)_tcslen(szText), 0);

        if (!pStyle && (szText[0] == '<'))
        {
            // This is an error message.  Do not draw the value on the right.
        }
		else
		{
			//Draw the style bytes
			if ((dis->itemState & ODS_SELECTED))
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			else
				SetTextColor(dis->hDC, GetSysColor(COLOR_3DSHADOW));

			if (pStyle)
				_stprintf_s(szText, ARRAYSIZE(szText), szHexFmt, pStyle->value); // otherwise, this is the "unrecognized bits" item and its text coincides with its numeric value

			dis->rcItem.right -= 4;

			DrawText(dis->hDC, szText, -1, &dis->rcItem, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

			dis->rcItem.right += 4;
		}

		SetTextColor(dis->hDC, crFG);
		SetBkColor(dis->hDC, crBG);

		if (dis->itemState & ODS_FOCUS)
			DrawFocusRect(dis->hDC, &dis->rcItem);

		break;
	}


	return TRUE;
}
