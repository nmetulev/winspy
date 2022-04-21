//
//  Poster Dialog.
//
//  Copyright (c)
//  Freeware
//
//  Just a simple dialog which allows you to
//  send / post messages to a window
//

#include "WinSpy.h"

#include "resource.h"
#include "Utils.h"

typedef struct {
    PCWSTR pszMsgName;
    UINT uMsgValue;
} MessageLookup;

// Based on WinUser.h
static const MessageLookup WindowMessages[] = {
    L"WM_NULL", 0x0000,
    L"WM_CREATE", 0x0001,
    L"WM_DESTROY", 0x0002,
    L"WM_MOVE", 0x0003,
    L"WM_SIZE", 0x0005,
    L"WM_ACTIVATE", 0x0006,
    L"WM_SETFOCUS", 0x0007,
    L"WM_KILLFOCUS", 0x0008,
    L"WM_ENABLE", 0x000A,
    L"WM_SETREDRAW", 0x000B,
    L"WM_SETTEXT", 0x000C,
    L"WM_GETTEXT", 0x000D,
    L"WM_GETTEXTLENGTH", 0x000E,
    L"WM_PAINT", 0x000F,
    L"WM_CLOSE", 0x0010,
    L"WM_QUERYENDSESSION", 0x0011,
    L"WM_QUERYOPEN", 0x0013,
    L"WM_ENDSESSION", 0x0016,
    L"WM_QUIT", 0x0012,
    L"WM_ERASEBKGND", 0x0014,
    L"WM_SYSCOLORCHANGE", 0x0015,
    L"WM_SHOWWINDOW", 0x0018,
    L"WM_WININICHANGE", 0x001A,
    L"WM_SETTINGCHANGE", 0x001A,
    L"WM_DEVMODECHANGE", 0x001B,
    L"WM_ACTIVATEAPP", 0x001C,
    L"WM_FONTCHANGE", 0x001D,
    L"WM_TIMECHANGE", 0x001E,
    L"WM_CANCELMODE", 0x001F,
    L"WM_SETCURSOR", 0x0020,
    L"WM_MOUSEACTIVATE", 0x0021,
    L"WM_CHILDACTIVATE", 0x0022,
    L"WM_QUEUESYNC", 0x0023,
    L"WM_GETMINMAXINFO", 0x0024,
    L"WM_PAINTICON", 0x0026,
    L"WM_ICONERASEBKGND", 0x0027,
    L"WM_NEXTDLGCTL", 0x0028,
    L"WM_SPOOLERSTATUS", 0x002A,
    L"WM_DRAWITEM", 0x002B,
    L"WM_MEASUREITEM", 0x002C,
    L"WM_DELETEITEM", 0x002D,
    L"WM_VKEYTOITEM", 0x002E,
    L"WM_CHARTOITEM", 0x002F,
    L"WM_SETFONT", 0x0030,
    L"WM_GETFONT", 0x0031,
    L"WM_SETHOTKEY", 0x0032,
    L"WM_GETHOTKEY", 0x0033,
    L"WM_QUERYDRAGICON", 0x0037,
    L"WM_COMPAREITEM", 0x0039,
    L"WM_GETOBJECT", 0x003D,
    L"WM_COMPACTING", 0x0041,
    L"WM_COMMNOTIFY", 0x0044,
    L"WM_WINDOWPOSCHANGING", 0x0046,
    L"WM_WINDOWPOSCHANGED", 0x0047,
    L"WM_POWER", 0x0048,
    L"WM_COPYDATA", 0x004A,
    L"WM_CANCELJOURNAL", 0x004B,
    L"WM_NOTIFY", 0x004E,
    L"WM_INPUTLANGCHANGEREQUEST", 0x0050,
    L"WM_INPUTLANGCHANGE", 0x0051,
    L"WM_TCARD", 0x0052,
    L"WM_HELP", 0x0053,
    L"WM_USERCHANGED", 0x0054,
    L"WM_NOTIFYFORMAT", 0x0055,
    L"WM_CONTEXTMENU", 0x007B,
    L"WM_STYLECHANGING", 0x007C,
    L"WM_STYLECHANGED", 0x007D,
    L"WM_DISPLAYCHANGE", 0x007E,
    L"WM_GETICON", 0x007F,
    L"WM_SETICON", 0x0080,
    L"WM_NCCREATE", 0x0081,
    L"WM_NCDESTROY", 0x0082,
    L"WM_NCCALCSIZE", 0x0083,
    L"WM_NCHITTEST", 0x0084,
    L"WM_NCPAINT", 0x0085,
    L"WM_NCACTIVATE", 0x0086,
    L"WM_GETDLGCODE", 0x0087,
    L"WM_SYNCPAINT", 0x0088,
    L"WM_NCMOUSEMOVE", 0x00A0,
    L"WM_NCLBUTTONDOWN", 0x00A1,
    L"WM_NCLBUTTONUP", 0x00A2,
    L"WM_NCLBUTTONDBLCLK", 0x00A3,
    L"WM_NCRBUTTONDOWN", 0x00A4,
    L"WM_NCRBUTTONUP", 0x00A5,
    L"WM_NCRBUTTONDBLCLK", 0x00A6,
    L"WM_NCMBUTTONDOWN", 0x00A7,
    L"WM_NCMBUTTONUP", 0x00A8,
    L"WM_NCMBUTTONDBLCLK", 0x00A9,
    L"WM_NCXBUTTONDOWN", 0x00AB,
    L"WM_NCXBUTTONUP", 0x00AC,
    L"WM_NCXBUTTONDBLCLK", 0x00AD,
    L"WM_INPUT_DEVICE_CHANGE", 0x00FE,
    L"WM_INPUT", 0x00FF,
    L"WM_KEYFIRST", 0x0100,
    L"WM_KEYDOWN", 0x0100,
    L"WM_KEYUP", 0x0101,
    L"WM_CHAR", 0x0102,
    L"WM_DEADCHAR", 0x0103,
    L"WM_SYSKEYDOWN", 0x0104,
    L"WM_SYSKEYUP", 0x0105,
    L"WM_SYSCHAR", 0x0106,
    L"WM_SYSDEADCHAR", 0x0107,
    L"WM_UNICHAR", 0x0109,
    L"WM_KEYLAST", 0x0109,
    L"WM_KEYLAST", 0x0108,
    L"WM_IME_STARTCOMPOSITION", 0x010D,
    L"WM_IME_ENDCOMPOSITION", 0x010E,
    L"WM_IME_COMPOSITION", 0x010F,
    L"WM_IME_KEYLAST", 0x010F,
    L"WM_INITDIALOG", 0x0110,
    L"WM_COMMAND", 0x0111,
    L"WM_SYSCOMMAND", 0x0112,
    L"WM_TIMER", 0x0113,
    L"WM_HSCROLL", 0x0114,
    L"WM_VSCROLL", 0x0115,
    L"WM_INITMENU", 0x0116,
    L"WM_INITMENUPOPUP", 0x0117,
    L"WM_GESTURE", 0x0119,
    L"WM_GESTURENOTIFY", 0x011A,
    L"WM_MENUSELECT", 0x011F,
    L"WM_MENUCHAR", 0x0120,
    L"WM_ENTERIDLE", 0x0121,
    L"WM_MENURBUTTONUP", 0x0122,
    L"WM_MENUDRAG", 0x0123,
    L"WM_MENUGETOBJECT", 0x0124,
    L"WM_UNINITMENUPOPUP", 0x0125,
    L"WM_MENUCOMMAND", 0x0126,
    L"WM_CHANGEUISTATE", 0x0127,
    L"WM_UPDATEUISTATE", 0x0128,
    L"WM_QUERYUISTATE", 0x0129,
    L"WM_CTLCOLORMSGBOX", 0x0132,
    L"WM_CTLCOLOREDIT", 0x0133,
    L"WM_CTLCOLORLISTBOX", 0x0134,
    L"WM_CTLCOLORBTN", 0x0135,
    L"WM_CTLCOLORDLG", 0x0136,
    L"WM_CTLCOLORSCROLLBAR", 0x0137,
    L"WM_CTLCOLORSTATIC", 0x0138,
    L"MN_GETHMENU", 0x01E1,
    L"WM_MOUSEFIRST", 0x0200,
    L"WM_MOUSEMOVE", 0x0200,
    L"WM_LBUTTONDOWN", 0x0201,
    L"WM_LBUTTONUP", 0x0202,
    L"WM_LBUTTONDBLCLK", 0x0203,
    L"WM_RBUTTONDOWN", 0x0204,
    L"WM_RBUTTONUP", 0x0205,
    L"WM_RBUTTONDBLCLK", 0x0206,
    L"WM_MBUTTONDOWN", 0x0207,
    L"WM_MBUTTONUP", 0x0208,
    L"WM_MBUTTONDBLCLK", 0x0209,
    L"WM_MOUSEWHEEL", 0x020A,
    L"WM_XBUTTONDOWN", 0x020B,
    L"WM_XBUTTONUP", 0x020C,
    L"WM_XBUTTONDBLCLK", 0x020D,
    L"WM_MOUSEHWHEEL", 0x020E,
    L"WM_MOUSELAST", 0x020E,
    L"WM_MOUSELAST", 0x020D,
    L"WM_MOUSELAST", 0x020A,
    L"WM_MOUSELAST", 0x0209,
    L"WM_PARENTNOTIFY", 0x0210,
    L"WM_ENTERMENULOOP", 0x0211,
    L"WM_EXITMENULOOP", 0x0212,
    L"WM_NEXTMENU", 0x0213,
    L"WM_SIZING", 0x0214,
    L"WM_CAPTURECHANGED", 0x0215,
    L"WM_MOVING", 0x0216,
    L"WM_POWERBROADCAST", 0x0218,
    L"WM_DEVICECHANGE", 0x0219,
    L"WM_MDICREATE", 0x0220,
    L"WM_MDIDESTROY", 0x0221,
    L"WM_MDIACTIVATE", 0x0222,
    L"WM_MDIRESTORE", 0x0223,
    L"WM_MDINEXT", 0x0224,
    L"WM_MDIMAXIMIZE", 0x0225,
    L"WM_MDITILE", 0x0226,
    L"WM_MDICASCADE", 0x0227,
    L"WM_MDIICONARRANGE", 0x0228,
    L"WM_MDIGETACTIVE", 0x0229,
    L"WM_MDISETMENU", 0x0230,
    L"WM_ENTERSIZEMOVE", 0x0231,
    L"WM_EXITSIZEMOVE", 0x0232,
    L"WM_DROPFILES", 0x0233,
    L"WM_MDIREFRESHMENU", 0x0234,
    L"WM_TOUCH", 0x0240,
    L"WM_IME_SETCONTEXT", 0x0281,
    L"WM_IME_NOTIFY", 0x0282,
    L"WM_IME_CONTROL", 0x0283,
    L"WM_IME_COMPOSITIONFULL", 0x0284,
    L"WM_IME_SELECT", 0x0285,
    L"WM_IME_CHAR", 0x0286,
    L"WM_IME_REQUEST", 0x0288,
    L"WM_IME_KEYDOWN", 0x0290,
    L"WM_IME_KEYUP", 0x0291,
    L"WM_MOUSEHOVER", 0x02A1,
    L"WM_MOUSELEAVE", 0x02A3,
    L"WM_NCMOUSEHOVER", 0x02A0,
    L"WM_NCMOUSELEAVE", 0x02A2,
    L"WM_WTSSESSION_CHANGE", 0x02B1,
    L"WM_TABLET_FIRST", 0x02c0,
    L"WM_TABLET_LAST", 0x02df,
    L"WM_CUT", 0x0300,
    L"WM_COPY", 0x0301,
    L"WM_PASTE", 0x0302,
    L"WM_CLEAR", 0x0303,
    L"WM_UNDO", 0x0304,
    L"WM_RENDERFORMAT", 0x0305,
    L"WM_RENDERALLFORMATS", 0x0306,
    L"WM_DESTROYCLIPBOARD", 0x0307,
    L"WM_DRAWCLIPBOARD", 0x0308,
    L"WM_PAINTCLIPBOARD", 0x0309,
    L"WM_VSCROLLCLIPBOARD", 0x030A,
    L"WM_SIZECLIPBOARD", 0x030B,
    L"WM_ASKCBFORMATNAME", 0x030C,
    L"WM_CHANGECBCHAIN", 0x030D,
    L"WM_HSCROLLCLIPBOARD", 0x030E,
    L"WM_QUERYNEWPALETTE", 0x030F,
    L"WM_PALETTEISCHANGING", 0x0310,
    L"WM_PALETTECHANGED", 0x0311,
    L"WM_HOTKEY", 0x0312,
    L"WM_PRINT", 0x0317,
    L"WM_PRINTCLIENT", 0x0318,
    L"WM_APPCOMMAND", 0x0319,
    L"WM_THEMECHANGED", 0x031A,
    L"WM_CLIPBOARDUPDATE", 0x031D,
    L"WM_DWMCOMPOSITIONCHANGED", 0x031E,
    L"WM_DWMNCRENDERINGCHANGED", 0x031F,
    L"WM_DWMCOLORIZATIONCOLORCHANGED", 0x0320,
    L"WM_DWMWINDOWMAXIMIZEDCHANGE", 0x0321,
    L"WM_DWMSENDICONICTHUMBNAIL", 0x0323,
    L"WM_DWMSENDICONICLIVEPREVIEWBITMAP", 0x0326,
    L"WM_GETTITLEBARINFOEX", 0x033F,
    L"WM_HANDHELDFIRST", 0x0358,
    L"WM_HANDHELDLAST", 0x035F,
    L"WM_AFXFIRST", 0x0360,
    L"WM_AFXLAST", 0x037F,
    L"WM_PENWINFIRST", 0x0380,
    L"WM_PENWINLAST", 0x038F,
    L"WM_APP", 0x8000,
    L"WM_USER", 0x0400,
    L"EM_GETSEL", 0x00B0,
    L"EM_SETSEL", 0x00B1,
    L"EM_GETRECT", 0x00B2,
    L"EM_SETRECT", 0x00B3,
    L"EM_SETRECTNP", 0x00B4,
    L"EM_SCROLL", 0x00B5,
    L"EM_LINESCROLL", 0x00B6,
    L"EM_SCROLLCARET", 0x00B7,
    L"EM_GETMODIFY", 0x00B8,
    L"EM_SETMODIFY", 0x00B9,
    L"EM_GETLINECOUNT", 0x00BA,
    L"EM_LINEINDEX", 0x00BB,
    L"EM_SETHANDLE", 0x00BC,
    L"EM_GETHANDLE", 0x00BD,
    L"EM_GETTHUMB", 0x00BE,
    L"EM_LINELENGTH", 0x00C1,
    L"EM_REPLACESEL", 0x00C2,
    L"EM_GETLINE", 0x00C4,
    L"EM_LIMITTEXT", 0x00C5,
    L"EM_CANUNDO", 0x00C6,
    L"EM_UNDO", 0x00C7,
    L"EM_FMTLINES", 0x00C8,
    L"EM_LINEFROMCHAR", 0x00C9,
    L"EM_SETTABSTOPS", 0x00CB,
    L"EM_SETPASSWORDCHAR", 0x00CC,
    L"EM_EMPTYUNDOBUFFER", 0x00CD,
    L"EM_GETFIRSTVISIBLELINE", 0x00CE,
    L"EM_SETREADONLY", 0x00CF,
    L"EM_SETWORDBREAKPROC", 0x00D0,
    L"EM_GETWORDBREAKPROC", 0x00D1,
    L"EM_GETPASSWORDCHAR", 0x00D2,
    L"EM_SETMARGINS", 0x00D3,
    L"EM_GETMARGINS", 0x00D4,
    L"EM_SETLIMITTEXT", 0x00C5,
    L"EM_GETLIMITTEXT", 0x00D5,
    L"EM_POSFROMCHAR", 0x00D6,
    L"EM_CHARFROMPOS", 0x00D7,
    L"EM_SETIMESTATUS", 0x00D8,
    L"EM_GETIMESTATUS", 0x00D9,
    L"BM_GETCHECK", 0x00F0,
    L"BM_SETCHECK", 0x00F1,
    L"BM_GETSTATE", 0x00F2,
    L"BM_SETSTATE", 0x00F3,
    L"BM_SETSTYLE", 0x00F4,
    L"BM_CLICK", 0x00F5,
    L"BM_GETIMAGE", 0x00F6,
    L"BM_SETIMAGE", 0x00F7,
    L"BM_SETDONTCLICK", 0x00F8,
    L"STM_SETICON", 0x0170,
    L"STM_GETICON", 0x0171,
    L"STM_SETIMAGE", 0x0172,
    L"STM_GETIMAGE", 0x0173,
    L"STM_MSGMAX", 0x0174,
    L"LB_ADDSTRING", 0x0180,
    L"LB_INSERTSTRING", 0x0181,
    L"LB_DELETESTRING", 0x0182,
    L"LB_SELITEMRANGEEX", 0x0183,
    L"LB_RESETCONTENT", 0x0184,
    L"LB_SETSEL", 0x0185,
    L"LB_SETCURSEL", 0x0186,
    L"LB_GETSEL", 0x0187,
    L"LB_GETCURSEL", 0x0188,
    L"LB_GETTEXT", 0x0189,
    L"LB_GETTEXTLEN", 0x018A,
    L"LB_GETCOUNT", 0x018B,
    L"LB_SELECTSTRING", 0x018C,
    L"LB_DIR", 0x018D,
    L"LB_GETTOPINDEX", 0x018E,
    L"LB_FINDSTRING", 0x018F,
    L"LB_GETSELCOUNT", 0x0190,
    L"LB_GETSELITEMS", 0x0191,
    L"LB_SETTABSTOPS", 0x0192,
    L"LB_GETHORIZONTALEXTENT", 0x0193,
    L"LB_SETHORIZONTALEXTENT", 0x0194,
    L"LB_SETCOLUMNWIDTH", 0x0195,
    L"LB_ADDFILE", 0x0196,
    L"LB_SETTOPINDEX", 0x0197,
    L"LB_GETITEMRECT", 0x0198,
    L"LB_GETITEMDATA", 0x0199,
    L"LB_SETITEMDATA", 0x019A,
    L"LB_SELITEMRANGE", 0x019B,
    L"LB_SETANCHORINDEX", 0x019C,
    L"LB_GETANCHORINDEX", 0x019D,
    L"LB_SETCARETINDEX", 0x019E,
    L"LB_GETCARETINDEX", 0x019F,
    L"LB_SETITEMHEIGHT", 0x01A0,
    L"LB_GETITEMHEIGHT", 0x01A1,
    L"LB_FINDSTRINGEXACT", 0x01A2,
    L"LB_SETLOCALE", 0x01A5,
    L"LB_GETLOCALE", 0x01A6,
    L"LB_SETCOUNT", 0x01A7,
    L"LB_INITSTORAGE", 0x01A8,
    L"LB_ITEMFROMPOINT", 0x01A9,
    L"LB_MULTIPLEADDSTRING", 0x01B1,
    L"LB_GETLISTBOXINFO", 0x01B2,
    L"LB_MSGMAX", 0x01B3,
    L"LB_MSGMAX", 0x01B1,
    L"LB_MSGMAX", 0x01B0,
    L"LB_MSGMAX", 0x01A8,
    L"CB_GETEDITSEL", 0x0140,
    L"CB_LIMITTEXT", 0x0141,
    L"CB_SETEDITSEL", 0x0142,
    L"CB_ADDSTRING", 0x0143,
    L"CB_DELETESTRING", 0x0144,
    L"CB_DIR", 0x0145,
    L"CB_GETCOUNT", 0x0146,
    L"CB_GETCURSEL", 0x0147,
    L"CB_GETLBTEXT", 0x0148,
    L"CB_GETLBTEXTLEN", 0x0149,
    L"CB_INSERTSTRING", 0x014A,
    L"CB_RESETCONTENT", 0x014B,
    L"CB_FINDSTRING", 0x014C,
    L"CB_SELECTSTRING", 0x014D,
    L"CB_SETCURSEL", 0x014E,
    L"CB_SHOWDROPDOWN", 0x014F,
    L"CB_GETITEMDATA", 0x0150,
    L"CB_SETITEMDATA", 0x0151,
    L"CB_GETDROPPEDCONTROLRECT", 0x0152,
    L"CB_SETITEMHEIGHT", 0x0153,
    L"CB_GETITEMHEIGHT", 0x0154,
    L"CB_SETEXTENDEDUI", 0x0155,
    L"CB_GETEXTENDEDUI", 0x0156,
    L"CB_GETDROPPEDSTATE", 0x0157,
    L"CB_FINDSTRINGEXACT", 0x0158,
    L"CB_SETLOCALE", 0x0159,
    L"CB_GETLOCALE", 0x015A,
    L"CB_GETTOPINDEX", 0x015b,
    L"CB_SETTOPINDEX", 0x015c,
    L"CB_GETHORIZONTALEXTENT", 0x015d,
    L"CB_SETHORIZONTALEXTENT", 0x015e,
    L"CB_GETDROPPEDWIDTH", 0x015f,
    L"CB_SETDROPPEDWIDTH", 0x0160,
    L"CB_INITSTORAGE", 0x0161,
    L"CB_MULTIPLEADDSTRING", 0x0163,
    L"CB_GETCOMBOBOXINFO", 0x0164,
    L"CB_MSGMAX", 0x0165,
    L"CB_MSGMAX", 0x0163,
    L"CB_MSGMAX", 0x0162,
    L"CB_MSGMAX", 0x015B,
    L"SBM_SETPOS", 0x00E0,
    L"SBM_GETPOS", 0x00E1,
    L"SBM_SETRANGE", 0x00E2,
    L"SBM_SETRANGEREDRAW", 0x00E6,
    L"SBM_GETRANGE", 0x00E3,
    L"SBM_ENABLE_ARROWS", 0x00E4,
    L"SBM_SETSCROLLINFO", 0x00E9,
    L"SBM_GETSCROLLINFO", 0x00EA,
    L"SBM_GETSCROLLBARINFO", 0x00EB,
    NULL, 0
};

//
// Checks if window is valid for Poster
//
static BOOL IsWindowEx(HWND hwnd)
{
    return hwnd == HWND_BROADCAST || IsWindow(hwnd);
}

static void SetInitialGuiInfo(HWND hwnd, HWND hwndTarget)
{
    HWND     hwndMsgsCombo;
    WCHAR    ach[256];

    _stprintf_s(ach, ARRAYSIZE(ach), L"%08X", (UINT)(UINT_PTR)hwndTarget);
    SetDlgItemText(hwnd, IDC_POSTER_HANDLE, ach);

    hwndMsgsCombo = GetDlgItem(hwnd, IDC_POSTER_MESSAGES);
    for (int i = 0; WindowMessages[i].pszMsgName; i++)
    {
        int index = (int)SendMessage(hwndMsgsCombo, CB_ADDSTRING, 0, (LPARAM)WindowMessages[i].pszMsgName);
        if (index != -1)
        {
            SendMessage(hwndMsgsCombo, CB_SETITEMDATA, index, WindowMessages[i].uMsgValue);
        }
    }
}

static void GetGuiInfo(HWND hwnd, HWND *phwndTarget, UINT *puMsg, WPARAM *pwParam, LPARAM *plParam)
{
    HWND     hwndMsgsCombo;
    int      nComboIndex;

    *phwndTarget = (HWND)GetDlgItemBaseInt(hwnd, IDC_POSTER_HANDLE, 16);

    hwndMsgsCombo = GetDlgItem(hwnd, IDC_POSTER_MESSAGES);
    nComboIndex = (int)SendMessage(hwndMsgsCombo, CB_GETCURSEL, 0, 0);
    if (nComboIndex != -1)
    {
        *puMsg = (UINT)SendMessage(hwndMsgsCombo, CB_GETITEMDATA, nComboIndex, 0);
    }
    else
    {
        *puMsg = (UINT)GetDlgItemBaseInt(hwnd, IDC_POSTER_MESSAGES, 16);
    }

    *pwParam = (WPARAM)GetDlgItemBaseInt(hwnd, IDC_POSTER_WPARAM, 16);
    *plParam = (LPARAM)GetDlgItemBaseInt(hwnd, IDC_POSTER_LPARAM, 16);
}

static void PosterSendMessage(HWND hwnd)
{
    HWND     hwndTarget;
    UINT     uMsg;
    WPARAM   wParam;
    LPARAM   lParam;
    WCHAR    ach[256];
    DWORD_PTR dwResult;

    GetGuiInfo(hwnd, &hwndTarget, &uMsg, &wParam, &lParam);

    if (!IsWindowEx(hwndTarget))
    {
        MessageBox(hwnd,
            L"Not a valid window",
            szAppName,
            MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    if (SendMessageTimeout(hwndTarget, uMsg, wParam, lParam, 0, 7000, &dwResult))
    {
        _stprintf_s(ach, ARRAYSIZE(ach), L"%p", (void*)dwResult);
    }
    else
    {
        _stprintf_s(ach, ARRAYSIZE(ach), L"Error 0x%08X", GetLastError());
    }

    SetDlgItemText(hwnd, IDC_POSTER_RESULT, ach);
}

static void PosterPostMessage(HWND hwnd)
{
    HWND     hwndTarget;
    UINT     uMsg;
    WPARAM   wParam;
    LPARAM   lParam;
    WCHAR    ach[256];

    GetGuiInfo(hwnd, &hwndTarget, &uMsg, &wParam, &lParam);

    if (!IsWindowEx(hwndTarget))
    {
        MessageBox(hwnd,
            L"Not a valid window",
            szAppName,
            MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    if (PostMessage(hwndTarget, uMsg, wParam, lParam))
    {
        wcscpy_s(ach, ARRAYSIZE(ach), L"Posted");
    }
    else
    {
        _stprintf_s(ach, ARRAYSIZE(ach), L"Error 0x%08X", GetLastError());
    }

    SetDlgItemText(hwnd, IDC_POSTER_RESULT, ach);
}

//
//  Dialog procedure for the poster window
//
INT_PTR CALLBACK PosterDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndTarget;    // target window!

    switch (iMsg)
    {
    case WM_INITDIALOG:
        hwndTarget = (HWND)lParam;

        SetInitialGuiInfo(hwnd, hwndTarget);
        return TRUE;

    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_POSTER_SEND:
            PosterSendMessage(hwnd);
            return TRUE;

        case IDC_POSTER_POST:
            PosterPostMessage(hwnd);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
        return FALSE;
    }

    return FALSE;
}


void ShowPosterDlg(HWND hwndParent, HWND hwndTarget)
{
    if (IsWindow(spy_hCurWnd))
    {
        DialogBoxParam(
            hInst,
            MAKEINTRESOURCE(IDD_POSTER),
            hwndParent,
            PosterDlgProc,
            (LPARAM)hwndTarget);

        SetGeneralInfo(hwndTarget);
    }
    else
    {
        MessageBox(hwndParent,
            L"Not a valid window",
            szAppName,
            MB_OK | MB_ICONEXCLAMATION);
    }
}


void ShowBroadcasterDlg(HWND hwndParent)
{
    DialogBoxParam(
        hInst,
        MAKEINTRESOURCE(IDD_POSTER),
        hwndParent,
        PosterDlgProc,
        (LPARAM)HWND_BROADCAST);
}

