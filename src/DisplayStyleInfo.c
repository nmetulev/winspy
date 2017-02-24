//
//	DisplayStyleInfo.c
//  Copyright (c) 2002 by J Brown 
//	Freeware
//
//  void SetStyleInfo(HWND hwnd)
//
//	 Fill the style-tab-pane with style info for the
//   specified window
//
//  void FillStyleLists(HWND hwndTarget, HWND hwndStyleList, 
//					BOOL fAllStyles, DWORD dwStyles)
//
//  void FillExStyleLists(HWND hwndTarget, HWND hwndExStyleList, 
//					BOOL fAllStyles, DWORD dwExStyles, BOOL fExtControl)
//
//	 Fill the listbox with the appropriate style strings
//   based on dw[Ex]Styles for the specified target window.
//
//	 hwndTarget      - window to find styles for
//	 hwndStyleList   - listbox to receive standard styles
//   hwndExStyleList - listbox to receive extended styles
//  
//   fAllStyles      - FALSE - just adds the styles that are present in dw[Ex]Styles
//                     TRUE  - adds ALL possible styles, but
//                             only selects those that are present
//
//   dw[Ex]Styles    - the styles value
//
//   fExtControl     - include control-specific extended styles
//                     (e.g. LVS_EX_xxx styles), not present in
//                     dwStyleEx
//
//
//	v1.6.1 - fixed small bug thanks to Holger Stenger
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>
#include <richedit.h>
#include "resource.h"
#include "WinSpy.h"

StyleLookupEx WindowStyles[] =
{
	NAMEANDVALUE_(WS_OVERLAPPEDWINDOW),	0, -1, (WS_POPUP | WS_CHILD), // WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
	NAMEANDVALUE_(WS_POPUPWINDOW),		WS_POPUPWINDOW, -1, 0, // WS_POPUP | WS_BORDER | WS_SYSMENU

	//{ 0xC0000000
	NAMEANDVALUE_(WS_OVERLAPPED),		0, -1, (WS_POPUP | WS_CHILD),	//0x00000000
	NAMEANDVALUE_(WS_POPUP),			0, -1, 0,						//0x80000000
	NAMEANDVALUE_(WS_CHILD),			0, -1, 0,						//0x40000000
	//} 0xC0000000
	NAMEANDVALUE_(WS_MINIMIZE),			0, -1, 0,						//0x20000000
	NAMEANDVALUE_(WS_VISIBLE),			0, -1, 0,						//0x10000000
	NAMEANDVALUE_(WS_DISABLED),			0, -1, 0,						//0x08000000
	NAMEANDVALUE_(WS_CLIPSIBLINGS),		0, -1, 0,						//0x04000000
	NAMEANDVALUE_(WS_CLIPCHILDREN),		0, -1, 0,						//0x02000000
	NAMEANDVALUE_(WS_MAXIMIZE),			0, -1, 0,						//0x01000000
	//{ 0x00C00000
	NAMEANDVALUE_(WS_CAPTION),			0, -1, 0,						//0x00C00000 /* WS_BORDER | WS_DLGFRAME  */
	NAMEANDVALUE_(WS_BORDER),			0, -1, 0,						//0x00800000
	NAMEANDVALUE_(WS_DLGFRAME),			0, -1, 0,						//0x00400000
	//} 0x00C00000
	NAMEANDVALUE_(WS_VSCROLL),			0, -1, 0,						//0x00200000
	NAMEANDVALUE_(WS_HSCROLL),			0, -1, 0,						//0x00100000
	NAMEANDVALUE_(WS_SYSMENU),			0, -1, 0,						//0x00080000
	NAMEANDVALUE_(WS_THICKFRAME),		0, -1, 0,						//0x00040000
	NAMEANDVALUE_(WS_GROUP),			0, -1, 0,						//0x00020000
	NAMEANDVALUE_(WS_TABSTOP),			0, -1, 0,						//0x00010000

	NAMEANDVALUE_(WS_MINIMIZEBOX),		0, WS_POPUPWINDOW | WS_OVERLAPPEDWINDOW | WS_CAPTION, 0, //0x00020000
	NAMEANDVALUE_(WS_MAXIMIZEBOX),		0, WS_POPUPWINDOW | WS_OVERLAPPEDWINDOW | WS_CAPTION, 0, //0x00010000

	NULL
};

// Dialog box styles (class = #32770)
StyleLookupEx DialogStyles[] =
{
	NAMEANDVALUE_(DS_ABSALIGN),			0, -1, 0,			//0x00000001
	NAMEANDVALUE_(DS_SYSMODAL),			0, -1, 0,			//0x00000002
	NAMEANDVALUE_(DS_LOCALEDIT),		0, -1, 0,			//0x00000020
	NAMEANDVALUE_(DS_SETFONT),			0, -1, 0,			//0x00000040
	NAMEANDVALUE_(DS_MODALFRAME),		0, -1, 0,			//0x00000080
	NAMEANDVALUE_(DS_NOIDLEMSG),		0, -1, 0,			//0x00000100
	NAMEANDVALUE_(DS_SETFOREGROUND),	0, -1, 0,			//0x00000200

#if(WINVER >= 0x0400)

	NAMEANDVALUE_(DS_3DLOOK),			0, -1, 0,			//0x00000004
	NAMEANDVALUE_(DS_FIXEDSYS),			0, -1, 0,			//0x00000008
	NAMEANDVALUE_(DS_NOFAILCREATE),		0, -1, 0,			//0x00000010
	NAMEANDVALUE_(DS_CONTROL),			0, -1, 0,			//0x00000400
	NAMEANDVALUE_(DS_CENTER),			0, -1, 0,			//0x00000800
	NAMEANDVALUE_(DS_CENTERMOUSE),		0, -1, 0,			//0x00001000
	NAMEANDVALUE_(DS_CONTEXTHELP),		0, -1, 0,			//0x00002000

#endif

	NULL
};

// Button styles (Button)
StyleLookupEx ButtonStyles[] =
{
	NAMEANDVALUE_(BS_PUSHBUTTON),		0,   -1, BS_DEFPUSHBUTTON | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_RADIOBUTTON | BS_GROUPBOX | BS_AUTORADIOBUTTON,
	NAMEANDVALUE_(BS_DEFPUSHBUTTON),	0xf, -1, 0,			//0x0001
	NAMEANDVALUE_(BS_CHECKBOX),			0xf, -1, 0,			//0x0002
	NAMEANDVALUE_(BS_AUTOCHECKBOX),		0xf, -1, 0,			//0x0003
	NAMEANDVALUE_(BS_RADIOBUTTON),		0xf, -1, 0,			//0x0004
	NAMEANDVALUE_(BS_3STATE),			0xf, -1, 0,			//0x0005
	NAMEANDVALUE_(BS_AUTO3STATE),		0xf, -1, 0,			//0x0006
	NAMEANDVALUE_(BS_GROUPBOX),			0xf, -1, 0,			//0x0007
	NAMEANDVALUE_(BS_USERBUTTON),		0xf, -1, 0,			//0x0008
	NAMEANDVALUE_(BS_AUTORADIOBUTTON),	0xf, -1, 0,			//0x0009
	NAMEANDVALUE_(BS_OWNERDRAW),		0xf, -1, 0,			//0x000B
	NAMEANDVALUE_(BS_LEFTTEXT),			0,   -1, 0,			//0x0020

	//winver >= 4.0 (index 42)
	NAMEANDVALUE_(BS_TEXT),				0, -1, (BS_ICON | BS_BITMAP | BS_AUTOCHECKBOX | BS_AUTORADIOBUTTON | BS_CHECKBOX | BS_RADIOBUTTON),//0x00000000
	NAMEANDVALUE_(BS_ICON),				0, -1, 0,			//0x0040	
	NAMEANDVALUE_(BS_BITMAP),			0, -1, 0,			//0x0080
	NAMEANDVALUE_(BS_LEFT),				0, -1, 0,			//0x0100
	NAMEANDVALUE_(BS_RIGHT),			0, -1, 0,			//0x0200
	NAMEANDVALUE_(BS_CENTER),			0, -1, 0,			//0x0300
	NAMEANDVALUE_(BS_TOP),				0, -1, 0,			//0x0400
	NAMEANDVALUE_(BS_BOTTOM),			0, -1, 0,			//0x0800
	NAMEANDVALUE_(BS_VCENTER),			0, -1, 0,			//0x0C00
	NAMEANDVALUE_(BS_PUSHLIKE),			0, -1, 0,			//0x1000
	NAMEANDVALUE_(BS_MULTILINE),		0, -1, 0,			//0x2000
	NAMEANDVALUE_(BS_NOTIFY),			0, -1, 0,			//0x4000
	NAMEANDVALUE_(BS_FLAT),				0, -1, 0,			//0x8000
	NAMEANDVALUE_(BS_RIGHTBUTTON),		0, -1, 0,			//BS_LEFTTEXT

	NULL
};

// Edit styles (Edit)
StyleLookupEx EditStyles[] =
{
	NAMEANDVALUE_(ES_LEFT),				0, -1, (ES_CENTER | ES_RIGHT),	//0x0000
	NAMEANDVALUE_(ES_CENTER),			0, -1, 0,						//0x0001
	NAMEANDVALUE_(ES_RIGHT),			0, -1, 0,						//0x0002
	NAMEANDVALUE_(ES_MULTILINE),		0, -1, 0,						//0x0004
	NAMEANDVALUE_(ES_UPPERCASE),		0, -1, 0,						//0x0008
	NAMEANDVALUE_(ES_LOWERCASE),		0, -1, 0,						//0x0010
	NAMEANDVALUE_(ES_PASSWORD),			0, -1, 0,						//0x0020
	NAMEANDVALUE_(ES_AUTOVSCROLL),		0, -1, 0,						//0x0040
	NAMEANDVALUE_(ES_AUTOHSCROLL),		0, -1, 0,						//0x0080
	NAMEANDVALUE_(ES_NOHIDESEL),		0, -1, 0,						//0x0100
	NAMEANDVALUE_(ES_OEMCONVERT),		0, -1, 0,						//0x0400
	NAMEANDVALUE_(ES_READONLY),			0, -1, 0,						//0x0800
	NAMEANDVALUE_(ES_WANTRETURN),		0, -1, 0,						//0x1000
	NAMEANDVALUE_(ES_NUMBER),			0, -1, 0,						//0x2000	

	NULL
};

StyleLookupEx RichedStyles[] =
{
	// Standard edit control styles
	NAMEANDVALUE_(ES_LEFT),				0, -1, (ES_CENTER | ES_RIGHT),	//0x0000
	NAMEANDVALUE_(ES_CENTER),			0, -1, 0,						//0x0001
	NAMEANDVALUE_(ES_RIGHT),			0, -1, 0,						//0x0002
	NAMEANDVALUE_(ES_MULTILINE),		0, -1, 0,						//0x0004
	//NAMEANDVALUE_(ES_UPPERCASE),		0, -1, 0,						//0x0008
	//NAMEANDVALUE_(ES_LOWERCASE),		0, -1, 0,						//0x0010
	NAMEANDVALUE_(ES_PASSWORD),			0, -1, 0,						//0x0020
	NAMEANDVALUE_(ES_AUTOVSCROLL),		0, -1, 0,						//0x0040
	NAMEANDVALUE_(ES_AUTOHSCROLL),		0, -1, 0,						//0x0080
	NAMEANDVALUE_(ES_NOHIDESEL),		0, -1, 0,						//0x0100
	//NAMEANDVALUE_(ES_OEMCONVERT),		0, -1, 0,						//0x0400
	NAMEANDVALUE_(ES_READONLY),			0, -1, 0,						//0x0800
	NAMEANDVALUE_(ES_WANTRETURN),		0, -1, 0,						//0x1000
	NAMEANDVALUE_(ES_NUMBER),			0, -1, 0,						//0x2000	

	// Additional Rich Edit control styles

	NAMEANDVALUE_(ES_SAVESEL),			0, -1, 0,				//0x00008000
	NAMEANDVALUE_(ES_SUNKEN),			0, -1, 0,				//0x00004000
	NAMEANDVALUE_(ES_DISABLENOSCROLL),	0, -1, 0,				//0x00002000
	NAMEANDVALUE_(ES_SELECTIONBAR),		0, -1, 0,				//0x01000000
	NAMEANDVALUE_(ES_NOOLEDRAGDROP),	0, -1, 0,				//0x00000008

	NULL

};

// Combo box styles (combobox)
StyleLookupEx ComboStyles[] =
{
	NAMEANDVALUE_(CBS_SIMPLE),				0x3, -1, 0,		//0x0001
	NAMEANDVALUE_(CBS_DROPDOWN),			0x3, -1, 0,		//0x0002
	NAMEANDVALUE_(CBS_DROPDOWNLIST),		0x3, -1, 0,		//0x0003
	NAMEANDVALUE_(CBS_OWNERDRAWFIXED),		0, -1, 0,		//0x0010
	NAMEANDVALUE_(CBS_OWNERDRAWVARIABLE),	0, -1, 0,		//0x0020
	NAMEANDVALUE_(CBS_AUTOHSCROLL),			0, -1, 0,		//0x0040
	NAMEANDVALUE_(CBS_OEMCONVERT),			0, -1, 0,		//0x0080
	NAMEANDVALUE_(CBS_SORT),				0, -1, 0,		//0x0100
	NAMEANDVALUE_(CBS_HASSTRINGS),			0, -1, 0,		//0x0200
	NAMEANDVALUE_(CBS_NOINTEGRALHEIGHT),	0, -1, 0,		//0x0400
	NAMEANDVALUE_(CBS_DISABLENOSCROLL),		0, -1, 0,		//0x0800

#if(WINVER >= 0x0400)
	NAMEANDVALUE_(CBS_UPPERCASE),			0, -1, 0,		//0x1000
	NAMEANDVALUE_(CBS_LOWERCASE),			0, -1, 0,		//0x2000
#endif

	NULL
};

// Listbox styles (Listbox)
StyleLookupEx ListBoxStyles[] =
{
	NAMEANDVALUE_(LBS_NOTIFY),				0, -1, 0,		//0x0001
	NAMEANDVALUE_(LBS_SORT),				0, -1, 0,		//0x0002
	NAMEANDVALUE_(LBS_NOREDRAW),			0, -1, 0,		//0x0004
	NAMEANDVALUE_(LBS_MULTIPLESEL),			0, -1, 0,		//0x0008
	NAMEANDVALUE_(LBS_OWNERDRAWFIXED),		0, -1, 0,		//0x0010
	NAMEANDVALUE_(LBS_OWNERDRAWVARIABLE),	0, -1, 0,		//0x0020
	NAMEANDVALUE_(LBS_HASSTRINGS),			0, -1, 0,		//0x0040
	NAMEANDVALUE_(LBS_USETABSTOPS),			0, -1, 0,		//0x0080
	NAMEANDVALUE_(LBS_NOINTEGRALHEIGHT),	0, -1, 0,		//0x0100
	NAMEANDVALUE_(LBS_MULTICOLUMN),			0, -1, 0,		//0x0200
	NAMEANDVALUE_(LBS_WANTKEYBOARDINPUT),	0, -1, 0,		//0x0400
	NAMEANDVALUE_(LBS_EXTENDEDSEL),			0, -1, 0,		//0x0800
	NAMEANDVALUE_(LBS_DISABLENOSCROLL),		0, -1, 0,		//0x1000
	NAMEANDVALUE_(LBS_NODATA),				0, -1, 0,		//0x2000
	NAMEANDVALUE_(LBS_NOSEL),				0, -1, 0,		//0x4000

	NULL
};

// Scrollbar control styles (Scrollbar)
StyleLookupEx ScrollbarStyles[] =
{
	NAMEANDVALUE_(SBS_TOPALIGN),				0, SBS_HORZ, 0,								//0x0002
	NAMEANDVALUE_(SBS_LEFTALIGN),				0, SBS_VERT, 0,								//0x0002
	NAMEANDVALUE_(SBS_BOTTOMALIGN),				0, SBS_HORZ | SBS_SIZEBOX | SBS_SIZEGRIP, 0,	//0x0004
	NAMEANDVALUE_(SBS_RIGHTALIGN),				0, SBS_VERT | SBS_SIZEBOX | SBS_SIZEGRIP, 0,	//0x0004
	NAMEANDVALUE_(SBS_HORZ),					0, -1, SBS_VERT | SBS_SIZEBOX | SBS_SIZEGRIP,	//0x0000
	NAMEANDVALUE_(SBS_VERT),					0, -1, SBS_SIZEBOX | SBS_SIZEGRIP,			//0x0001
	NAMEANDVALUE_(SBS_SIZEBOXTOPLEFTALIGN),		0, SBS_SIZEBOX | SBS_SIZEGRIP, 0,				//0x0002
	NAMEANDVALUE_(SBS_SIZEBOXBOTTOMRIGHTALIGN),	0, SBS_SIZEBOX | SBS_SIZEGRIP, 0,				//0x0004
	NAMEANDVALUE_(SBS_SIZEBOX),					0, -1, 0,									//0x0008
	NAMEANDVALUE_(SBS_SIZEGRIP),				0, -1, 0,									//0x0010

	NULL
};

// Static control styles (Static)
StyleLookupEx StaticStyles[] =
{
	NAMEANDVALUE_(SS_LEFT),					0x1f, -1,SS_CENTER | SS_RIGHT,//0x0000
	NAMEANDVALUE_(SS_CENTER),				0x1f, -1, 0,				//0x0001
	NAMEANDVALUE_(SS_RIGHT),				0x1f, -1, 0,				//0x0002
	NAMEANDVALUE_(SS_ICON),					0x1f, -1, 0,				//0x0003
	NAMEANDVALUE_(SS_BLACKRECT),			0x1f, -1, 0,				//0x0004
	NAMEANDVALUE_(SS_GRAYRECT),				0x1f, -1, 0,				//0x0005
	NAMEANDVALUE_(SS_WHITERECT),			0x1f, -1, 0,				//0x0006
	NAMEANDVALUE_(SS_BLACKFRAME),			0x1f, -1, 0,				//0x0007
	NAMEANDVALUE_(SS_GRAYFRAME),			0x1f, -1, 0,				//0x0008
	NAMEANDVALUE_(SS_WHITEFRAME),			0x1f, -1, 0,				//0x0009
	NAMEANDVALUE_(SS_USERITEM),				0x1f, -1, 0,				//0x000A
	NAMEANDVALUE_(SS_SIMPLE),				0x1f, -1, 0,				//0x000B
	NAMEANDVALUE_(SS_LEFTNOWORDWRAP),		0x1f, -1, 0,				//0x000C

	NAMEANDVALUE_(SS_OWNERDRAW),			0x1f, -1, 0,				//0x000D
	NAMEANDVALUE_(SS_BITMAP),				0x1f, -1, 0,				//0x000E
	NAMEANDVALUE_(SS_ENHMETAFILE),			0x1f, -1, 0,				//0x000F
	NAMEANDVALUE_(SS_ETCHEDHORZ),			0x1f, -1, 0,				//0x0010
	NAMEANDVALUE_(SS_ETCHEDVERT),			0x1f, -1, 0,				//0x0011
	NAMEANDVALUE_(SS_ETCHEDFRAME),			0x1f, -1, 0,				//0x0012
	NAMEANDVALUE_(SS_TYPEMASK),				0x1f, -1, 0,				//0x001F
	NAMEANDVALUE_(SS_NOPREFIX),				0,    -1, 0,				//0x0080

	NAMEANDVALUE_(SS_NOTIFY),				0,    -1, 0,				//0x0100
	NAMEANDVALUE_(SS_CENTERIMAGE),			0,    -1, 0,				//0x0200
	NAMEANDVALUE_(SS_RIGHTJUST),			0,    -1, 0,				//0x0400
	NAMEANDVALUE_(SS_REALSIZEIMAGE),		0,    -1, 0,				//0x0800
	NAMEANDVALUE_(SS_SUNKEN),				0,    -1, 0,				//0x1000
	NAMEANDVALUE_(SS_ENDELLIPSIS),			0,    -1, 0,				//0x4000
	NAMEANDVALUE_(SS_PATHELLIPSIS),			0,    -1, 0,				//0x8000
	NAMEANDVALUE_(SS_WORDELLIPSIS),			0,    -1, 0,				//0xC000
	NAMEANDVALUE_(SS_ELLIPSISMASK),			0,    -1, 0,				//0xC000

	NULL
};

//	Standard Common controls styles	
StyleLookupEx CommCtrlList[] =
{
	NAMEANDVALUE_(CCS_TOP),					0x3, -1, 0,			//0x0001
	NAMEANDVALUE_(CCS_NOMOVEY),				0x3, -1, 0,			//0x0002
	NAMEANDVALUE_(CCS_BOTTOM),				0x3, -1, 0,			//0x0003
	NAMEANDVALUE_(CCS_NORESIZE),			0,	 -1, 0,			//0x0004
	NAMEANDVALUE_(CCS_NOPARENTALIGN),		0,   -1, 0,			//0x0008

	NAMEANDVALUE_(CCS_ADJUSTABLE),			0,   -1, 0,			//0x0020
	NAMEANDVALUE_(CCS_NODIVIDER),			0,   -1, 0,			//0x0040

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(CCS_VERT),				0,   -1, 0,			//0x0080
#endif

	NULL
};

//  DragList - uses same styles as listview

// Header control (SysHeader32)
StyleLookupEx HeaderStyles[] =
{
	NAMEANDVALUE_(HDS_HORZ),				0, -1, 16,			//0x0000
	NAMEANDVALUE_(HDS_BUTTONS),				0, -1, 0,			//0x0002

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(HDS_HOTTRACK),			0, -1, 0,			//0x0004
	NAMEANDVALUE_(HDS_DRAGDROP),			0, -1, 0,			//0x0040
	NAMEANDVALUE_(HDS_FULLDRAG),			0, -1, 0,			//0x0080
#endif

	NAMEANDVALUE_(HDS_HIDDEN),				0, -1, 0,			//0x0008

#if (_WIN32_IE >= 0x0500)
	NAMEANDVALUE_(HDS_FILTERBAR),			0, -1, 0,			//0x0100
#endif

	NULL
};

// Listview (SysListView32)
StyleLookupEx ListViewStyles[] =
{
	NAMEANDVALUE_(LVS_ICON),				LVS_TYPEMASK, -1, LVS_REPORT | LVS_SMALLICON | LVS_LIST, //0x0000
	NAMEANDVALUE_(LVS_REPORT),				LVS_TYPEMASK, -1, 0,		//0x0001
	NAMEANDVALUE_(LVS_SMALLICON),			LVS_TYPEMASK, -1, 0,		//0x0002
	NAMEANDVALUE_(LVS_LIST),				LVS_TYPEMASK, -1, 0,		//0x0003

	NAMEANDVALUE_(LVS_SINGLESEL),			0,   -1, 0,		//0x0004
	NAMEANDVALUE_(LVS_SHOWSELALWAYS),		0,   -1, 0,		//0x0008
	NAMEANDVALUE_(LVS_SORTASCENDING),		0,   -1, 0,		//0x0010
	NAMEANDVALUE_(LVS_SORTDESCENDING),		0,   -1, 0,		//0x0020
	NAMEANDVALUE_(LVS_SHAREIMAGELISTS),		0,   -1, 0,		//0x0040
	NAMEANDVALUE_(LVS_NOLABELWRAP),			0,   -1, 0,		//0x0080
	NAMEANDVALUE_(LVS_AUTOARRANGE),			0,   -1, 0,		//0x0100
	NAMEANDVALUE_(LVS_EDITLABELS),			0,   -1, 0,		//0x0200

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(LVS_OWNERDATA),			0, -1, 0,		//0x1000
#endif

	NAMEANDVALUE_(LVS_NOSCROLL),			0, -1, 0,		//0x2000

	NAMEANDVALUE_(LVS_ALIGNTOP),			0, -1, 0,		//0x0000
	NAMEANDVALUE_(LVS_ALIGNLEFT),			LVS_ALIGNMASK, -1, 0,	//0x0800

	//NAMEANDVALUE_(LVS_ALIGNMASK),			0, -1, 0,		//0x0c00
	//NAMEANDVALUE_(LVS_TYPESTYLEMASK),		0, -1, 0,		//0xfc00

	NAMEANDVALUE_(LVS_OWNERDRAWFIXED),		0, -1, 0,		//0x0400
	NAMEANDVALUE_(LVS_NOCOLUMNHEADER),		0, -1, 0,		//0x4000
	NAMEANDVALUE_(LVS_NOSORTHEADER),		0, -1, 0,		//0x8000

	NULL
};

// Toolbar control (ToolbarWindow32)
StyleLookupEx ToolbarStyles[] =
{
	NAMEANDVALUE_(TBSTYLE_TOOLTIPS),		0, -1, 0,		//0x0100
	NAMEANDVALUE_(TBSTYLE_WRAPABLE),		0, -1, 0,		//0x0200
	NAMEANDVALUE_(TBSTYLE_ALTDRAG),			0, -1, 0,		//0x0400

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(TBSTYLE_FLAT),			0, -1, 0,		//0x0800
	NAMEANDVALUE_(TBSTYLE_LIST),			0, -1, 0,		//0x1000
	NAMEANDVALUE_(TBSTYLE_CUSTOMERASE),		0, -1, 0,		//0x2000
#endif

#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(TBSTYLE_REGISTERDROP),	0, -1, 0,		//0x4000
	NAMEANDVALUE_(TBSTYLE_TRANSPARENT),		0, -1, 0,		//0x8000
#endif

	NULL
};

// Rebar control (RebarControl32)
StyleLookupEx RebarStyles[] =
{
#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(RBS_TOOLTIPS),			0, -1, 0,		//0x0100
	NAMEANDVALUE_(RBS_VARHEIGHT),			0, -1, 0,		//0x0200
	NAMEANDVALUE_(RBS_BANDBORDERS),			0, -1, 0,		//0x0400
	NAMEANDVALUE_(RBS_FIXEDORDER),			0, -1, 0,		//0x0800
	NAMEANDVALUE_(RBS_REGISTERDROP),		0, -1, 0,		//0x1000
	NAMEANDVALUE_(RBS_AUTOSIZE),			0, -1, 0,		//0x2000
	NAMEANDVALUE_(RBS_VERTICALGRIPPER),		0, -1, 0,		//0x4000
	NAMEANDVALUE_(RBS_DBLCLKTOGGLE),		0, -1, 0,		//0x8000
#endif

	NULL
};

// Track Bar control (msctls_trackbar32)
StyleLookupEx TrackbarStyles[] =
{
	NAMEANDVALUE_(TBS_AUTOTICKS),			0xf, -1, 0,				//0x0001
	NAMEANDVALUE_(TBS_VERT),				0xf, -1, 0,				//0x0002
	NAMEANDVALUE_(TBS_HORZ),				0xf, -1, TBS_VERT,		//0x0000
	NAMEANDVALUE_(TBS_TOP),					0xf, -1, 0,				//0x0004
	NAMEANDVALUE_(TBS_BOTTOM),				0xf, -1, TBS_TOP,		//0x0000
	NAMEANDVALUE_(TBS_LEFT),				0xf, -1, 0,				//0x0004
	NAMEANDVALUE_(TBS_RIGHT),				0xf, -1, TBS_LEFT,		//0x0000
	NAMEANDVALUE_(TBS_BOTH),				0xf, -1, 0,				//0x0008

	NAMEANDVALUE_(TBS_NOTICKS),				0, -1, 0,				//0x0010
	NAMEANDVALUE_(TBS_ENABLESELRANGE),		0, -1, 0,				//0x0020
	NAMEANDVALUE_(TBS_FIXEDLENGTH),			0, -1, 0,				//0x0040
	NAMEANDVALUE_(TBS_NOTHUMB),				0, -1, 0,				//0x0080

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(TBS_TOOLTIPS),			0, -1, 0,				//0x0100
#endif

#if (_WIN32_IE >= 0x0500)
	NAMEANDVALUE_(TBS_REVERSED),			0, -1, 0,				//0x0200  
#endif

	NULL
};

// Treeview (SysTreeView32)
StyleLookupEx TreeViewStyles[] =
{
	NAMEANDVALUE_(TVS_HASBUTTONS),			0, -1, 0,			//0x0001
	NAMEANDVALUE_(TVS_HASLINES),			0, -1, 0,			//0x0002
	NAMEANDVALUE_(TVS_LINESATROOT),			0, -1, 0,			//0x0004
	NAMEANDVALUE_(TVS_EDITLABELS),			0, -1, 0,			//0x0008
	NAMEANDVALUE_(TVS_DISABLEDRAGDROP),		0, -1, 0,			//0x0010
	NAMEANDVALUE_(TVS_SHOWSELALWAYS),		0, -1, 0,			//0x0020

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(TVS_RTLREADING),			0, -1, 0,			//0x0040
	NAMEANDVALUE_(TVS_NOTOOLTIPS),			0, -1, 0,			//0x0080
	NAMEANDVALUE_(TVS_CHECKBOXES),			0, -1, 0,			//0x0100
	NAMEANDVALUE_(TVS_TRACKSELECT),			0, -1, 0,			//0x0200

#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(TVS_SINGLEEXPAND),		0, -1, 0,			//0x0400
	NAMEANDVALUE_(TVS_INFOTIP),				0, -1, 0,			//0x0800
	NAMEANDVALUE_(TVS_FULLROWSELECT),		0, -1, 0,			//0x1000
	NAMEANDVALUE_(TVS_NOSCROLL),			0, -1, 0,			//0x2000
	NAMEANDVALUE_(TVS_NONEVENHEIGHT),		0, -1, 0,			//0x4000

#if (_WIN32_IE >= 0x500)
	NAMEANDVALUE_(TVS_NOHSCROLL),			0, -1, 0,			//0x8000

#endif
#endif
#endif

	NULL
};

// Tooltips (tooltips_class32)
StyleLookupEx ToolTipStyles[] =
{
	NAMEANDVALUE_(TTS_ALWAYSTIP),			0, -1, 0,			//0x01
	NAMEANDVALUE_(TTS_NOPREFIX),			0, -1, 0,			//0x02

#if (_WIN32_IE >= 0x0500)
	NAMEANDVALUE_(TTS_NOANIMATE),			0, -1, 0,			//0x10
	NAMEANDVALUE_(TTS_NOFADE),				0, -1, 0,			//0x20
	NAMEANDVALUE_(TTS_BALLOON),				0, -1, 0,			//0x40
#endif

	NULL
};

// Statusbar (msctls_statusbar32)
StyleLookupEx StatusBarStyles[] =
{
	NAMEANDVALUE_(SBARS_SIZEGRIP),			0, -1, 0,			//0x0100

#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(SBT_TOOLTIPS),			0, -1, 0,			//0x0800
#endif

	NULL
};

// Updown control
StyleLookupEx UpDownStyles[] =
{
	NAMEANDVALUE_(UDS_WRAP),				0, -1, 0,			//0x0001
	NAMEANDVALUE_(UDS_SETBUDDYINT),			0, -1, 0,			//0x0002
	NAMEANDVALUE_(UDS_ALIGNRIGHT),			0, -1, 0,			//0x0004
	NAMEANDVALUE_(UDS_ALIGNLEFT),			0, -1, 0,			//0x0008
	NAMEANDVALUE_(UDS_AUTOBUDDY),			0, -1, 0,			//0x0010
	NAMEANDVALUE_(UDS_ARROWKEYS),			0, -1, 0,			//0x0020
	NAMEANDVALUE_(UDS_HORZ),				0, -1, 0,			//0x0040
	NAMEANDVALUE_(UDS_NOTHOUSANDS),			0, -1, 0,			//0x0080

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(UDS_HOTTRACK),			0, -1, 0,			//0x0100
#endif

	NULL
};

// Progress control (msctls_progress32)
StyleLookupEx ProgressStyles[] =
{
#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(PBS_SMOOTH),				0, -1, 0,			//0x01
	NAMEANDVALUE_(PBS_VERTICAL),			0, -1, 0,			//0x04
#endif

	NULL
};

// Tab control (SysTabControl32)
StyleLookupEx TabStyles[] =
{
#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(TCS_SCROLLOPPOSITE),		0, -1, 0,			//0x0001   // assumes multiline tab
	NAMEANDVALUE_(TCS_BOTTOM),				0, TCS_VERTICAL, 0,	//0x0002
	NAMEANDVALUE_(TCS_RIGHT),				0, -1, 0,			//0x0002
	NAMEANDVALUE_(TCS_MULTISELECT),			0, -1, 0,			//0x0004  
#endif
#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(TCS_FLATBUTTONS),			0, -1, 0,			//0x0008
#endif
	NAMEANDVALUE_(TCS_FORCEICONLEFT),		0, -1, 0,			//0x0010
	NAMEANDVALUE_(TCS_FORCELABELLEFT),		0, -1, 0,			//0x0020
#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(TCS_HOTTRACK),			0, -1, 0,			//0x0040
	NAMEANDVALUE_(TCS_VERTICAL),			0, -1, 0,			//0x0080
#endif
	NAMEANDVALUE_(TCS_TABS),				0, -1,TCS_BUTTONS,	//0x0000
	NAMEANDVALUE_(TCS_BUTTONS),				0, -1, 0,			//0x0100
	NAMEANDVALUE_(TCS_SINGLELINE),			0, -1,TCS_MULTILINE,//0x0000
	NAMEANDVALUE_(TCS_MULTILINE),			0, -1, 0,			//0x0200
	NAMEANDVALUE_(TCS_RIGHTJUSTIFY),		0, -1, -1,			//0x0000
	NAMEANDVALUE_(TCS_FIXEDWIDTH),			0, -1, 0,			//0x0400
	NAMEANDVALUE_(TCS_RAGGEDRIGHT),			0, -1, 0,			//0x0800
	NAMEANDVALUE_(TCS_FOCUSONBUTTONDOWN),	0, -1, 0,			//0x1000
	NAMEANDVALUE_(TCS_OWNERDRAWFIXED),		0, -1, 0,			//0x2000
	NAMEANDVALUE_(TCS_TOOLTIPS),			0, -1, 0,			//0x4000
	NAMEANDVALUE_(TCS_FOCUSNEVER),			0, -1, 0,			//0x8000

	NULL
};

// Animation control (SysAnimate32)
StyleLookupEx AnimateStyles[] =
{
	NAMEANDVALUE_(ACS_CENTER),				0, -1, 0,			//0x0001
	NAMEANDVALUE_(ACS_TRANSPARENT),			0, -1, 0,			//0x0002
	NAMEANDVALUE_(ACS_AUTOPLAY),			0, -1, 0,			//0x0004

#if (_WIN32_IE >= 0x0300)
	NAMEANDVALUE_(ACS_TIMER),				0, -1, 0,			//0x0008
#endif

	NULL
};

// Month-calendar control (SysMonthCal32)
StyleLookupEx MonthCalStyles[] =
{
	NAMEANDVALUE_(MCS_DAYSTATE),			0, -1, 0,			//0x0001
	NAMEANDVALUE_(MCS_MULTISELECT),			0, -1, 0,			//0x0002
	NAMEANDVALUE_(MCS_WEEKNUMBERS),			0, -1, 0,			//0x0004

#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(MCS_NOTODAYCIRCLE),		0, -1, 0,			//0x0008
	NAMEANDVALUE_(MCS_NOTODAY),				0, -1, 0,			//0x0010
#endif

	NULL
};

// Date-Time picker (SysDateTimePick32)
StyleLookupEx DateTimeStyles[] =
{
	NAMEANDVALUE_(DTS_UPDOWN),					0, -1, 0,			//0x0001
	NAMEANDVALUE_(DTS_SHOWNONE),				0, -1, 0,			//0x0002
	NAMEANDVALUE_(DTS_SHORTDATEFORMAT),			0, -1, DTS_LONGDATEFORMAT,//0x0000
	NAMEANDVALUE_(DTS_LONGDATEFORMAT),			0, -1, 0,			//0x0004 

#if (_WIN32_IE >= 0x500)
	NAMEANDVALUE_(DTS_SHORTDATECENTURYFORMAT),	0, -1, 0,		//0x000C
#endif

	NAMEANDVALUE_(DTS_TIMEFORMAT),				0, -1, 0,			//0x0009 
	NAMEANDVALUE_(DTS_APPCANPARSE),				0, -1, 0,			//0x0010 
	NAMEANDVALUE_(DTS_RIGHTALIGN),				0, -1, 0,			//0x0020 

	NULL
};

// Pager control (SysPager)
StyleLookupEx PagerStyles[] =
{
	//Pager control
	NAMEANDVALUE_(PGS_VERT),					0, -1, PGS_HORZ,	//0x0000
	NAMEANDVALUE_(PGS_HORZ),					0, -1, 0,			//0x0001
	NAMEANDVALUE_(PGS_AUTOSCROLL),				0, -1, 0,			//0x0002
	NAMEANDVALUE_(PGS_DRAGNDROP),				0, -1, 0,			//0x0004

	NULL
};

// Extended window styles (for all windows)
StyleLookupEx StyleExList[] =
{
	NAMEANDVALUE_(WS_EX_DLGMODALFRAME),		0, -1, 0,	//0x00000001L
	NAMEANDVALUE_(WS_EX_NOPARENTNOTIFY),	0, -1, 0,	//0x00000004L
	NAMEANDVALUE_(WS_EX_TOPMOST),			0, -1, 0,	//0x00000008L
	NAMEANDVALUE_(WS_EX_ACCEPTFILES),		0, -1, 0,	//0x00000010L
	NAMEANDVALUE_(WS_EX_TRANSPARENT),		0, -1, 0,	//0x00000020L

#if(WINVER >= 0x0400)

	NAMEANDVALUE_(WS_EX_MDICHILD),			0, -1, 0,	//0x00000040L
	NAMEANDVALUE_(WS_EX_TOOLWINDOW),		0, -1, 0,	//0x00000080L
	NAMEANDVALUE_(WS_EX_WINDOWEDGE),		0, -1, 0,	//0x00000100L
	NAMEANDVALUE_(WS_EX_CLIENTEDGE),		0, -1, 0,	//0x00000200L
	NAMEANDVALUE_(WS_EX_CONTEXTHELP),		0, -1, 0,	//0x00000400L

	NAMEANDVALUE_(WS_EX_LEFT),				0, -1, (WS_EX_RIGHT),		//0x00000000L
	NAMEANDVALUE_(WS_EX_RIGHT),				0, -1, 0,	//0x00001000L
	NAMEANDVALUE_(WS_EX_LTRREADING),		0, -1, (WS_EX_RTLREADING),	//0x00000000L
	NAMEANDVALUE_(WS_EX_RTLREADING),		0, -1, 0,	//0x00002000L
	NAMEANDVALUE_(WS_EX_LEFTSCROLLBAR),		0, -1, 0,	//0x00004000L
	NAMEANDVALUE_(WS_EX_RIGHTSCROLLBAR),	0, -1, (WS_EX_LEFTSCROLLBAR),//0x00000000L

	NAMEANDVALUE_(WS_EX_CONTROLPARENT),		0, -1, 0,	//0x00010000L
	NAMEANDVALUE_(WS_EX_STATICEDGE),		0, -1, 0,	//0x00020000L
	NAMEANDVALUE_(WS_EX_APPWINDOW),			0, -1, 0,	//0x00040000L

	NAMEANDVALUE_(WS_EX_OVERLAPPEDWINDOW),	0, -1, 0,	//(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
	//NAMEANDVALUE_(WS_EX_PALETTEWINDOW),	0, -1, 0,	//(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

#endif

#if(_WIN32_WINNT >= 0x0500)
	NAMEANDVALUE_(WS_EX_LAYERED),              0, -1, 0,   //0x00080000

#endif /* _WIN32_WINNT >= 0x0500 */


#if(WINVER >= 0x0500)
	NAMEANDVALUE_(WS_EX_NOINHERITLAYOUT),      0, -1, 0,   //0x00100000L // Disable inheritance of mirroring by children
	NAMEANDVALUE_(WS_EX_LAYOUTRTL),            0, -1, 0,   //0x00400000L // Right to left mirroring
#endif /* WINVER >= 0x0500 */

#if(_WIN32_WINNT >= 0x0501)
	NAMEANDVALUE_(WS_EX_COMPOSITED),           0, -1, 0,   //0x02000000L
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0500)
	NAMEANDVALUE_(WS_EX_NOACTIVATE),           0, -1, 0,   //0x08000000L
#endif /* _WIN32_WINNT >= 0x0500 */

	NULL
};

// ListView extended styles
StyleLookupEx ListViewExStyles[] =
{
	//ListView control styles
	NAMEANDVALUE_(LVS_EX_GRIDLINES),			0, -1, 0,		//0x00000001
	NAMEANDVALUE_(LVS_EX_SUBITEMIMAGES),		0, -1, 0,		//0x00000002
	NAMEANDVALUE_(LVS_EX_CHECKBOXES),			0, -1, 0,		//0x00000004
	NAMEANDVALUE_(LVS_EX_TRACKSELECT),			0, -1, 0,		//0x00000008
	NAMEANDVALUE_(LVS_EX_HEADERDRAGDROP),		0, -1, 0,		//0x00000010
	NAMEANDVALUE_(LVS_EX_FULLROWSELECT),		0, -1, 0,		//0x00000020
	NAMEANDVALUE_(LVS_EX_ONECLICKACTIVATE),		0, -1, 0,		//0x00000040
	NAMEANDVALUE_(LVS_EX_TWOCLICKACTIVATE),		0, -1, 0,		//0x00000080
#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(LVS_EX_FLATSB),				0, -1, 0,		//0x00000100
	NAMEANDVALUE_(LVS_EX_REGIONAL),				0, -1, 0,		//0x00000200
	NAMEANDVALUE_(LVS_EX_INFOTIP),				0, -1, 0,		//0x00000400
	NAMEANDVALUE_(LVS_EX_UNDERLINEHOT),			0, -1, 0,		//0x00000800
	NAMEANDVALUE_(LVS_EX_UNDERLINECOLD),		0, -1, 0,		//0x00001000
	NAMEANDVALUE_(LVS_EX_MULTIWORKAREAS),		0, -1, 0,		//0x00002000
#endif
#if (_WIN32_IE >= 0x0500)
	NAMEANDVALUE_(LVS_EX_LABELTIP),				0, -1, 0,		//0x00004000
#endif

	NULL
};

// ComboBoxEx extended styles	
StyleLookupEx ComboBoxExStyles[] =
{
	NAMEANDVALUE_(CBES_EX_NOEDITIMAGE),			0, -1, 0,	//0x00000001
	NAMEANDVALUE_(CBES_EX_NOEDITIMAGEINDENT),	0, -1, 0,	//0x00000002
	NAMEANDVALUE_(CBES_EX_PATHWORDBREAKPROC),	0, -1, 0,	//0x00000004

#if(_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(CBES_EX_NOSIZELIMIT),			0, -1, 0,	//0x00000008
	NAMEANDVALUE_(CBES_EX_CASESENSITIVE),		0, -1, 0,	//0x00000010
#endif

	NULL
};

// Tab control extended styles
StyleLookupEx TabCtrlExStyles[] =
{
	NAMEANDVALUE_(TCS_EX_FLATSEPARATORS),		0, -1, 0,	//0x00000001
	NAMEANDVALUE_(TCS_EX_REGISTERDROP),			0, -1, 0,	//0x00000002
	NULL
};

// Toolbar extended styles
StyleLookupEx ToolBarExStyles[] =
{
#if (_WIN32_IE >= 0x0400)
	NAMEANDVALUE_(TBSTYLE_EX_DRAWDDARROWS),			0, -1, 0,	//0x0001

#if (_WIN32_IE >= 0x0501)
	NAMEANDVALUE_(TBSTYLE_EX_MIXEDBUTTONS),			0, -1, 0,	//0x0008
	NAMEANDVALUE_(TBSTYLE_EX_HIDECLIPPEDBUTTONS),	0, -1, 0,	//0x0010

#endif
#endif

	NULL
};

// Support RichEdit Event masks!!!
StyleLookupEx RichedEventMask[] =
{
	NAMEANDVALUE_(ENM_NONE),				0, -1,-1,	//0x00000000
	NAMEANDVALUE_(ENM_CHANGE),				0, -1, 0,	//0x00000001
	NAMEANDVALUE_(ENM_UPDATE),				0, -1, 0,	//0x00000002
	NAMEANDVALUE_(ENM_SCROLL),				0, -1, 0,	//0x00000004
	NAMEANDVALUE_(ENM_KEYEVENTS),			0, -1, 0,	//0x00010000
	NAMEANDVALUE_(ENM_MOUSEEVENTS),			0, -1, 0,	//0x00020000
	NAMEANDVALUE_(ENM_REQUESTRESIZE),		0, -1, 0,	//0x00040000
	NAMEANDVALUE_(ENM_SELCHANGE),			0, -1, 0,	//0x00080000
	NAMEANDVALUE_(ENM_DROPFILES),			0, -1, 0,	//0x00100000
	NAMEANDVALUE_(ENM_PROTECTED),			0, -1, 0,	//0x00200000
	NAMEANDVALUE_(ENM_CORRECTTEXT),			0, -1, 0,	//0x00400000		// PenWin specific
	NAMEANDVALUE_(ENM_SCROLLEVENTS),		0, -1, 0,	//0x00000008
	NAMEANDVALUE_(ENM_DRAGDROPDONE),		0, -1, 0,	//0x00000010

	// Far East specific notification mask
	NAMEANDVALUE_(ENM_IMECHANGE),			0, -1, 0,	//0x00800000		// unused by RE2.0
	NAMEANDVALUE_(ENM_LANGCHANGE),			0, -1, 0,	//0x01000000
	NAMEANDVALUE_(ENM_OBJECTPOSITIONS),		0, -1, 0,	//0x02000000
	NAMEANDVALUE_(ENM_LINK),				0, -1, 0,	//0x04000000

	NULL
};

//
//	Lookup table which matches window classnames to style-lists
//
ClassStyleLookup StandardControls[] =
{
	_T("#32770"), 				DialogStyles,		0,
	_T("Button"),				ButtonStyles,		0,
	_T("ComboBox"),				ComboStyles,		0,
	_T("Edit"),					EditStyles,			0,
	_T("ListBox"),				ListBoxStyles,		0,

	_T("RICHEDIT"),				RichedStyles,		0,
	_T("RichEdit20A"),			RichedStyles,		0,
	_T("RichEdit20W"),			RichedStyles,		0,

	_T("Scrollbar"),			ScrollbarStyles,	0,
	_T("Static"),				StaticStyles,		0,

	_T("SysAnimate32"),			AnimateStyles,		0,
	_T("ComboBoxEx"),			ComboStyles,		0,	//(Just a normal combobox)
	_T("SysDateTimePick32"),	DateTimeStyles,		0,
	_T("DragList"),				ListBoxStyles,		0,	//(Just a normal list)
	_T("SysHeader32"),			HeaderStyles,		0,
	//"SysIPAddress32",			IPAddressStyles,	0,	(NO STYLES)
	_T("SysListView32"),		ListViewStyles,		0,
	_T("SysMonthCal32"),		MonthCalStyles,		0,
	_T("SysPager"),				PagerStyles,		0,
	_T("msctls_progress32"),	ProgressStyles,		0,
	_T("RebarWindow32"),		RebarStyles,		0,
	_T("msctls_statusbar32"),	StatusBarStyles,	0,
	//"SysLink",				SysLinkStyles,		0,  (DO IT!)
	_T("SysTabControl32"),		TabStyles,			0,
	_T("ToolbarWindow32"),		ToolbarStyles,		0,
	_T("tooltips_class32"),		ToolTipStyles,		0,
	_T("msctls_trackbar32"),	TrackbarStyles,		0,
	_T("SysTreeView32"),		TreeViewStyles,		0,
	_T("msctls_updown32"),		UpDownStyles,		0,

	NULL
};

// Classes which use the CCS_xxx styles
ClassStyleLookup CustomControls[] =
{
	_T("msctls_statusbar32"),	CommCtrlList,		0,
	_T("RebarWindow32"),		CommCtrlList,		0,
	_T("ToolbarWindow32"),		CommCtrlList,		0,
	_T("SysHeader32"),			CommCtrlList,		0,

	NULL
};

// Classes which have extended window styles
ClassStyleLookup ExtendedControls[] =
{
	_T("SysTabControl32"),		TabCtrlExStyles,	TCM_GETEXTENDEDSTYLE,
	_T("ToolbarWindow32"),		ToolBarExStyles,	TB_GETEXTENDEDSTYLE,
	_T("ComboBox"),				ComboBoxExStyles,	CBEM_GETEXTENDEDSTYLE,
	_T("SysListView32"),		ListViewExStyles,	LVM_GETEXTENDEDLISTVIEWSTYLE,
	_T("RICHEDIT"),				RichedEventMask,    EM_GETEVENTMASK,
	_T("RichEdit20A"),			RichedEventMask,    EM_GETEVENTMASK,
	_T("RichEdit20W"),			RichedEventMask,    EM_GETEVENTMASK,

	NULL
};

//
// Match the window classname to a StyleLookupEx instance in a ClassStyleLookup table
//
// pClassList - a lookup table of classname / matching stylelist
// 
//
StyleLookupEx *FindStyleList(ClassStyleLookup *pClassList, TCHAR *szClassName, DWORD *pdwMessage)
{
	int i;

	for (i = 0; pClassList[i].stylelist; i++)
	{
		if (lstrcmpi(szClassName, pClassList[i].szClassName) == 0)
		{
			if (pdwMessage) *pdwMessage = pClassList[i].dwMessage;
			return pClassList[i].stylelist;
		}
	}

	return 0;
}

#define NUM_CLASS_STYLELISTS ARRAYSIZE(ClassStyleList)

//
//	Find all the styles that match from the specified list
//
//	StyleList  - list of styles
//  hwndList   - listbox to add styles to
//  dwStyles   - styles for the target window
//  fAllStyles - when true, add all known styles and select those present in the dwStyles value; otherwise, only add the ones present
//
DWORD EnumStyles(StyleLookupEx *StyleList, HWND hwndList, DWORD dwStyles, BOOL fAllStyles)
{
	// Remember what the dwStyles was before we start modifying it
	DWORD dwOrig = dwStyles;

	int            i, idx;
	BOOL           fAddIt;
	StyleLookupEx *pStyle;

	//
	//	Loop through all of the styles that we know about
	//	Check each style against our window's one, to see
	//  if it is set or not
	//
	for (i = 0; StyleList[i].name; i++)
	{
		fAddIt = FALSE;

		pStyle = &StyleList[i];

		// This style needs a mask to detect if it is set - 
		// i.e. the style doesn't just use one bit to decide if it is on/off.
		if (pStyle->cmp_mask != 0)
		{
			// Style is valid if the excludes styles are not set
			if (pStyle->excludes != 0 && (pStyle->excludes & (dwOrig & pStyle->cmp_mask)) == 0)
				fAddIt = TRUE;

			// If the style matches exactly (when masked)
			if (pStyle->style != 0 && (pStyle->cmp_mask & dwStyles) == pStyle->style)
				fAddIt = TRUE;
		}
		else
		{
			// Style is valid when 
			if (pStyle->excludes != 0 && (pStyle->excludes & dwOrig) == 0)
				fAddIt = TRUE;

			// If style matches exactly (all bits are set
			if (pStyle->style != 0 && (pStyle->style & dwStyles) == pStyle->style)
				fAddIt = TRUE;

			// If no bits are set, then we have to skip it
			else if (pStyle->style != 0 && (pStyle->style & dwStyles) == 0)
				fAddIt = FALSE;

			// If this style depends on others being set..
			if (dwStyles && (pStyle->depends & dwStyles) == 0)
				fAddIt = FALSE;
		}

		// Now add the style..
		if (fAddIt == TRUE || fAllStyles)
		{
			// We've added this style, so remove it to stop it appearing again
			if (fAddIt)
				dwStyles &= ~(pStyle->style);

			// Add to list, and set the list's extra item data to the style's data
			idx = (int)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)pStyle->name);
			SendMessage(hwndList, LB_SETITEMDATA, idx, (LPARAM)pStyle);

			if (fAllStyles)
				SendMessage(hwndList, LB_SETSEL, fAddIt, idx);
		}
	}

	// return the styles. This will be zero if we decoded all the bits
	// that were set, or non-zero if there are still bits left
	return dwStyles;
}

//
//  This function takes HWND of a ListBox, which we will fill
//  with the style strings based on dwStyles
//
void FillStyleLists(HWND hwndTarget, HWND hwndStyleList, BOOL fAllStyles, DWORD dwStyles)
{
	TCHAR szClassName[256];

	StyleLookupEx *StyleList;

	//window class
	GetClassName(hwndTarget, szClassName, ARRAYSIZE(szClassName));

	SendMessage(hwndStyleList, WM_SETREDRAW, FALSE, 0);

	// Empty the list
	SendMessage(hwndStyleList, LB_RESETCONTENT, 0, 0);

	// enumerate the standard window styles, for any window no 
	// matter what class it might be
	dwStyles = EnumStyles(WindowStyles, hwndStyleList, dwStyles, fAllStyles);

	// if the window class is one we know about, then see if we
	// can decode any more style bits
	// enumerate the custom control styles
	StyleList = FindStyleList(StandardControls, szClassName, 0);
	if (StyleList != 0)
		dwStyles = EnumStyles(StyleList, hwndStyleList, dwStyles, fAllStyles);

	// does the window support the CCS_xxx styles (custom control styles)
	StyleList = FindStyleList(CustomControls, szClassName, 0);
	if (StyleList != 0)
		dwStyles = EnumStyles(StyleList, hwndStyleList, dwStyles, fAllStyles);

	// if there are still style bits set in the window style,
	// then there is something that we can't decode. Just display
	// a single HEX entry at the end of the list.
	if (dwStyles != 0)
	{
		int idx;
		TCHAR ach[10];

		_stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, dwStyles);
		static_assert(ARRAYSIZE(ach) < MAX_STYLE_NAME_CCH, "Style name exceeds the expected limit");
		idx = (int)SendMessage(hwndStyleList, LB_ADDSTRING, 0, (LPARAM)ach);
		// For the "unrecognized bits" item, we don't store any StyleLookupEx item,
		// but its text coincides with the numeric value, so it will be used instead
		SendMessage(hwndStyleList, LB_SETITEMDATA, idx, 0);

		if (fAllStyles)
			SendMessage(hwndStyleList, LB_SETSEL, TRUE, idx);
	}

	SendMessage(hwndStyleList, WM_SETREDRAW, TRUE, 0);
}

//
//  This function takes HWND of a ListBox, which we will fill
//  with the extended style strings based on dwExStyles
//
void FillExStyleLists(HWND hwndTarget, HWND hwndExStyleList, BOOL fAllStyles, DWORD dwExStyles, BOOL fExtControl)
{
	TCHAR szClassName[256];
	DWORD dwMessage;

	StyleLookupEx *StyleList;

	//window class
	GetClassName(hwndTarget, szClassName, ARRAYSIZE(szClassName));

	SendMessage(hwndExStyleList, WM_SETREDRAW, FALSE, 0);

	// Empty the list
	SendMessage(hwndExStyleList, LB_RESETCONTENT, 0, 0);

	EnumStyles(StyleExList, hwndExStyleList, dwExStyles, fAllStyles);

	// Does this window use any custom control extended styles???
	// If it does, then dwMessage will contain the message identifier to send
	// to the window to retrieve them
	if (fExtControl)
	{
		StyleList = FindStyleList(ExtendedControls, szClassName, &dwMessage);

		// Add them if required
		if (StyleList != 0)
		{
			dwExStyles = (DWORD)SendMessage(hwndTarget, dwMessage, 0, 0);
			EnumStyles(StyleList, hwndExStyleList, dwExStyles, fAllStyles);
		}
	}

	SendMessage(hwndExStyleList, WM_SETREDRAW, TRUE, 0);
}

//
//	Update the Style tab with styles for specified window
//
void SetStyleInfo(HWND hwnd)
{
	TCHAR ach[12];
	DWORD dwStyles;
	DWORD dwExStyles;

	HWND hwndDlg = WinSpyTab[STYLE_TAB].hwnd;
	HWND hwndStyle, hwndStyleEx;

	if (hwnd == 0) return;

	// Display the window style in static label
	dwStyles = GetWindowLong(hwnd, GWL_STYLE);
	_stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, dwStyles);
	SetDlgItemText(hwndDlg, IDC_STYLE, ach);

	// Display the extended window style in static label
	dwExStyles = GetWindowLong(hwnd, GWL_EXSTYLE);
	_stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, dwExStyles);
	SetDlgItemText(hwndDlg, IDC_STYLEEX, ach);

	// Find handles to standard and extended style lists
	hwndStyle = GetDlgItem(hwndDlg, IDC_LIST1);
	hwndStyleEx = GetDlgItem(hwndDlg, IDC_LIST2);

	// Fill both lists with their styles!
	FillStyleLists(hwnd, hwndStyle, FALSE, dwStyles);
	FillExStyleLists(hwnd, hwndStyleEx, FALSE, dwExStyles, TRUE);
}
