//
//  DisplayStyleInfo.c
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  void SetStyleInfo(HWND hwnd)
//
//   Fill the style-tab-pane with style info for the
//   specified window
//
//  void FillStyleLists(HWND hwndTarget, HWND hwndStyleList,
//                  BOOL fAllStyles, DWORD dwStyles)
//
//  void FillExStyleLists(HWND hwndTarget, HWND hwndExStyleList,
//                  BOOL fAllStyles, DWORD dwExStyles, BOOL fExtControl)
//
//   Fill the listbox with the appropriate style strings
//   based on dw[Ex]Styles for the specified target window.
//
//   hwndTarget      - window to find styles for
//   hwndStyleList   - listbox to receive standard styles
//   hwndExStyleList - listbox to receive extended styles
//
//   fAllStyles      - FALSE - just adds the styles that are both applicable and present in dw[Ex]Styles
//                     TRUE  - adds ALL possible styles, but
//                             only selects those that are applicable and present
//
//   dw[Ex]Styles    - the styles value
//
//   fExtControl     - include control-specific extended styles
//                     (e.g. LVS_EX_xxx styles), not present in
//                     dwStyleEx
//
//
//  v1.6.1 - fixed small bug thanks to Holger Stenger
//

#include "WinSpy.h"

#include "Utils.h"
#include <richedit.h>
#include "resource.h"

StyleLookupEx WindowStyles[] =
{
    STYLE_COMBINATION_MASK(WS_OVERLAPPEDWINDOW, WS_OVERLAPPED_MASK),    // WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
    STYLE_COMBINATION_MASK(WS_POPUPWINDOW, WS_OVERLAPPED_MASK),         // WS_POPUP | WS_BORDER | WS_SYSMENU

    //{ WS_OVERLAPPED_MASK
    STYLE_MASK(WS_OVERLAPPED, WS_OVERLAPPED_MASK),                      //0x00000000
    STYLE_MASK(WS_POPUP, WS_OVERLAPPED_MASK),                           //0x80000000
    STYLE_MASK(WS_CHILD, WS_OVERLAPPED_MASK),                           //0x40000000
    //} WS_OVERLAPPED_MASK
    STYLE_SIMPLE(WS_MINIMIZE),                                          //0x20000000
    STYLE_SIMPLE(WS_VISIBLE),                                           //0x10000000
    STYLE_SIMPLE(WS_DISABLED),                                          //0x08000000
    STYLE_SIMPLE(WS_CLIPSIBLINGS),                                      //0x04000000
    STYLE_SIMPLE(WS_CLIPCHILDREN),                                      //0x02000000
    STYLE_SIMPLE(WS_MAXIMIZE),                                          //0x01000000

    STYLE_COMBINATION(WS_CAPTION),                                      //0x00C00000 /* WS_BORDER | WS_DLGFRAME  */

    STYLE_SIMPLE(WS_BORDER),                                            //0x00800000
    STYLE_SIMPLE(WS_DLGFRAME),                                          //0x00400000

    STYLE_SIMPLE(WS_VSCROLL),                                           //0x00200000
    STYLE_SIMPLE(WS_HSCROLL),                                           //0x00100000
    STYLE_SIMPLE(WS_SYSMENU),                                           //0x00080000
    STYLE_SIMPLE(WS_THICKFRAME),                                        //0x00040000
    STYLE_MASK_DEPENDS(WS_GROUP, 0, WS_CHILD, WS_OVERLAPPED_MASK),      //0x00020000
    STYLE_MASK_DEPENDS(WS_TABSTOP, 0, WS_CHILD, WS_OVERLAPPED_MASK),    //0x00010000

    STYLE_SIMPLE_DEPENDS(WS_MINIMIZEBOX, WS_SYSMENU),                   //0x00020000
    STYLE_SIMPLE_DEPENDS(WS_MAXIMIZEBOX, WS_SYSMENU),                   //0x00010000

    NULL
};

// Dialog box styles (class = #32770)
StyleLookupEx DialogStyles[] =
{
    STYLE_COMBINATION(DS_SHELLFONT),    //(DS_SETFONT | DS_FIXEDSYS)
    STYLE_SIMPLE(DS_ABSALIGN),          //0x00000001
    STYLE_SIMPLE(DS_SYSMODAL),          //0x00000002
    STYLE_SIMPLE(DS_3DLOOK),            //0x00000004
    STYLE_SIMPLE(DS_FIXEDSYS),          //0x00000008
    STYLE_SIMPLE(DS_NOFAILCREATE),      //0x00000010
    STYLE_SIMPLE(DS_LOCALEDIT),         //0x00000020
    STYLE_SIMPLE(DS_SETFONT),           //0x00000040
    STYLE_SIMPLE(DS_MODALFRAME),        //0x00000080
    STYLE_SIMPLE(DS_NOIDLEMSG),         //0x00000100
    STYLE_SIMPLE(DS_SETFOREGROUND),     //0x00000200
    STYLE_SIMPLE(DS_CONTROL),           //0x00000400
    STYLE_SIMPLE(DS_CENTER),            //0x00000800
    STYLE_SIMPLE(DS_CENTERMOUSE),       //0x00001000
    STYLE_SIMPLE(DS_CONTEXTHELP),       //0x00002000

    NULL
};

// Button styles (Button)
StyleLookupEx ButtonStyles[] =
{
    //{ BS_TYPEMASK
    STYLE_MASK(BS_PUSHBUTTON, BS_TYPEMASK),                 //0x0000
    STYLE_MASK(BS_DEFPUSHBUTTON, BS_TYPEMASK),              //0x0001
    STYLE_MASK(BS_CHECKBOX, BS_TYPEMASK),                   //0x0002
    STYLE_MASK(BS_AUTOCHECKBOX, BS_TYPEMASK),               //0x0003
    STYLE_MASK(BS_RADIOBUTTON, BS_TYPEMASK),                //0x0004
    STYLE_MASK(BS_3STATE, BS_TYPEMASK),                     //0x0005
    STYLE_MASK(BS_AUTO3STATE, BS_TYPEMASK),                 //0x0006
    STYLE_MASK(BS_GROUPBOX, BS_TYPEMASK),                   //0x0007
    STYLE_MASK(BS_USERBUTTON, BS_TYPEMASK),                 //0x0008
    STYLE_MASK(BS_AUTORADIOBUTTON, BS_TYPEMASK),            //0x0009
    STYLE_MASK(BS_OWNERDRAW, BS_TYPEMASK),                  //0x000B
    STYLE_MASK(BS_SPLITBUTTON, BS_TYPEMASK),                //0x000C
    STYLE_MASK(BS_DEFSPLITBUTTON, BS_TYPEMASK),             //0x000D
    STYLE_MASK(BS_COMMANDLINK, BS_TYPEMASK),                //0x000E
    STYLE_MASK(BS_DEFCOMMANDLINK, BS_TYPEMASK),             //0x000F
    //} BS_TYPEMASK

    STYLE_SIMPLE(BS_LEFTTEXT),                              //0x0020

    //{ BS_TEXT_MASK
    STYLE_MASK(BS_TEXT, BS_TEXT_MASK),                      //0x0000
    STYLE_MASK(BS_ICON, BS_TEXT_MASK),                      //0x0040
    STYLE_MASK(BS_BITMAP, BS_TEXT_MASK),                    //0x0080
    //} BS_TEXT_MASK
    STYLE_COMBINATION(BS_CENTER),                           //0x0300
    STYLE_SIMPLE(BS_LEFT),                                  //0x0100
    STYLE_SIMPLE(BS_RIGHT),                                 //0x0200
    STYLE_COMBINATION(BS_VCENTER),                          //0x0C00
    STYLE_SIMPLE(BS_TOP),                                   //0x0400
    STYLE_SIMPLE(BS_BOTTOM),                                //0x0800
    STYLE_SIMPLE(BS_PUSHLIKE),                              //0x1000
    STYLE_SIMPLE(BS_MULTILINE),                             //0x2000
    STYLE_SIMPLE(BS_NOTIFY),                                //0x4000
    STYLE_SIMPLE(BS_FLAT),                                  //0x8000
    STYLE_SIMPLE(BS_RIGHTBUTTON),                           //BS_LEFTTEXT

    NULL
};

// Edit styles (Edit)
StyleLookupEx EditStyles[] =
{
    STYLE_MASK(ES_LEFT, ES_CENTER | ES_RIGHT),              //0x0000
    STYLE_SIMPLE(ES_CENTER),                                //0x0001
    STYLE_SIMPLE(ES_RIGHT),                                 //0x0002
    STYLE_SIMPLE(ES_MULTILINE),                             //0x0004
    STYLE_SIMPLE(ES_UPPERCASE),                             //0x0008
    STYLE_SIMPLE(ES_LOWERCASE),                             //0x0010
    STYLE_SIMPLE(ES_PASSWORD),                              //0x0020
    STYLE_SIMPLE(ES_AUTOVSCROLL),                           //0x0040
    STYLE_SIMPLE(ES_AUTOHSCROLL),                           //0x0080
    STYLE_SIMPLE(ES_NOHIDESEL),                             //0x0100
    STYLE_SIMPLE(ES_OEMCONVERT),                            //0x0400
    STYLE_SIMPLE(ES_READONLY),                              //0x0800
    STYLE_SIMPLE(ES_WANTRETURN),                            //0x1000
    STYLE_SIMPLE(ES_NUMBER),                                //0x2000

    NULL
};

StyleLookupEx RichedStyles[] =
{
    // Standard edit control styles
    STYLE_MASK(ES_LEFT, ES_CENTER | ES_RIGHT),              //0x0000
    STYLE_SIMPLE(ES_CENTER),                                //0x0001
    STYLE_SIMPLE(ES_RIGHT),                                 //0x0002
    STYLE_SIMPLE(ES_MULTILINE),                             //0x0004
    //STYLE_SIMPLE(ES_UPPERCASE),                           //0x0008
    //STYLE_SIMPLE(ES_LOWERCASE),                           //0x0010
    STYLE_SIMPLE(ES_PASSWORD),                              //0x0020
    STYLE_SIMPLE(ES_AUTOVSCROLL),                           //0x0040
    STYLE_SIMPLE(ES_AUTOHSCROLL),                           //0x0080
    STYLE_SIMPLE(ES_NOHIDESEL),                             //0x0100
    //STYLE_SIMPLE(ES_OEMCONVERT),                          //0x0400
    STYLE_SIMPLE(ES_READONLY),                              //0x0800
    STYLE_SIMPLE(ES_WANTRETURN),                            //0x1000
    STYLE_SIMPLE(ES_NUMBER),                                //0x2000

    // Additional Rich Edit control styles

    STYLE_SIMPLE(ES_SAVESEL),                               //0x00008000
    STYLE_SIMPLE(ES_SUNKEN),                                //0x00004000
    STYLE_SIMPLE(ES_DISABLENOSCROLL),                       //0x00002000
    // Same as WS_MAXIMIZE, but that doesn't make sense so we re-use the value
    STYLE_SIMPLE(ES_SELECTIONBAR),                          //0x01000000
    // Same as ES_UPPERCASE, but re-used to completely disable OLE drag'n'drop
    STYLE_SIMPLE(ES_NOOLEDRAGDROP),                         //0x00000008

    NULL

};

// Combo box styles (combobox)
StyleLookupEx ComboStyles[] =
{
    //{ CBS_TYPE_MASK
    STYLE_MASK(CBS_SIMPLE, CBS_TYPE_MASK),                  //0x0001
    STYLE_MASK(CBS_DROPDOWN, CBS_TYPE_MASK),                //0x0002
    STYLE_MASK(CBS_DROPDOWNLIST, CBS_TYPE_MASK),            //0x0003
    //} CBS_TYPE_MASK
    STYLE_SIMPLE(CBS_OWNERDRAWFIXED),                       //0x0010
    STYLE_SIMPLE(CBS_OWNERDRAWVARIABLE),                    //0x0020
    STYLE_SIMPLE(CBS_AUTOHSCROLL),                          //0x0040
    STYLE_SIMPLE(CBS_OEMCONVERT),                           //0x0080
    STYLE_SIMPLE(CBS_SORT),                                 //0x0100
    STYLE_SIMPLE(CBS_HASSTRINGS),                           //0x0200
    STYLE_SIMPLE(CBS_NOINTEGRALHEIGHT),                     //0x0400
    STYLE_SIMPLE(CBS_DISABLENOSCROLL),                      //0x0800

#if(WINVER >= 0x0400)
    STYLE_SIMPLE(CBS_UPPERCASE),                            //0x1000
    STYLE_SIMPLE(CBS_LOWERCASE),                            //0x2000
#endif

    NULL
};

// Listbox styles (Listbox)
StyleLookupEx ListBoxStyles[] =
{
    STYLE_SIMPLE(LBS_NOTIFY),                   //0x0001
    STYLE_SIMPLE(LBS_SORT),                     //0x0002
    STYLE_SIMPLE(LBS_NOREDRAW),                 //0x0004
    STYLE_SIMPLE(LBS_MULTIPLESEL),              //0x0008
    STYLE_SIMPLE(LBS_OWNERDRAWFIXED),           //0x0010
    STYLE_SIMPLE(LBS_OWNERDRAWVARIABLE),        //0x0020
    STYLE_SIMPLE(LBS_HASSTRINGS),               //0x0040
    STYLE_SIMPLE(LBS_USETABSTOPS),              //0x0080
    STYLE_SIMPLE(LBS_NOINTEGRALHEIGHT),         //0x0100
    STYLE_SIMPLE(LBS_MULTICOLUMN),              //0x0200
    STYLE_SIMPLE(LBS_WANTKEYBOARDINPUT),        //0x0400
    STYLE_SIMPLE(LBS_EXTENDEDSEL),              //0x0800
    STYLE_SIMPLE(LBS_DISABLENOSCROLL),          //0x1000
    STYLE_SIMPLE(LBS_NODATA),                   //0x2000
    STYLE_SIMPLE(LBS_NOSEL),                    //0x4000
    STYLE_SIMPLE(LBS_COMBOBOX),                 //0x8000

    NULL
};

// Scrollbar control styles (Scrollbar)
StyleLookupEx ScrollbarStyles[] =
{
    STYLE_MASK(SBS_HORZ, SBS_DIR_MASK),                             //0x0000
    STYLE_SIMPLE(SBS_VERT),                                         //0x0001
    STYLE_MASK_DEPENDS(SBS_TOPALIGN, 0, SBS_HORZ, SBS_DIR_MASK),    //0x0002
    STYLE_SIMPLE_DEPENDS(SBS_LEFTALIGN, SBS_VERT),                  //0x0002
    STYLE_MASK_DEPENDS(SBS_BOTTOMALIGN, 0, SBS_HORZ, SBS_DIR_MASK), //0x0004
    STYLE_SIMPLE_DEPENDS(SBS_RIGHTALIGN, SBS_VERT),                 //0x0004
    // SBS_SIZEBOXTOPLEFTALIGN and SBS_SIZEBOXBOTTOMRIGHTALIGN actually depend on
    // the presence of "either SBS_SIZEBOX or SBS_SIZEGRIP",
    // but our style definition format is not rich enough to express this.
    // It would be unjustified to complicate it just for this one case; also,
    // it would not allow for a meaningful "set style" definition for these styles anyway.
    // Therefore, we are ignoring this dependency and
    // defining these styles as ones without dependencies here.
    STYLE_SIMPLE(SBS_SIZEBOXTOPLEFTALIGN),                          //0x0002
    STYLE_SIMPLE(SBS_SIZEBOXBOTTOMRIGHTALIGN),                      //0x0004
    STYLE_SIMPLE(SBS_SIZEBOX),                                      //0x0008
    STYLE_SIMPLE(SBS_SIZEGRIP),                                     //0x0010

    NULL
};

// Static control styles (Static)
StyleLookupEx StaticStyles[] =
{
    //{ STYLE_MASK
    STYLE_MASK(SS_LEFT, SS_TYPEMASK),                   //0x0000
    STYLE_MASK(SS_CENTER, SS_TYPEMASK),                 //0x0001
    STYLE_MASK(SS_RIGHT, SS_TYPEMASK),                  //0x0002
    STYLE_MASK(SS_ICON, SS_TYPEMASK),                   //0x0003
    STYLE_MASK(SS_BLACKRECT, SS_TYPEMASK),              //0x0004
    STYLE_MASK(SS_GRAYRECT, SS_TYPEMASK),               //0x0005
    STYLE_MASK(SS_WHITERECT, SS_TYPEMASK),              //0x0006
    STYLE_MASK(SS_BLACKFRAME, SS_TYPEMASK),             //0x0007
    STYLE_MASK(SS_GRAYFRAME, SS_TYPEMASK),              //0x0008
    STYLE_MASK(SS_WHITEFRAME, SS_TYPEMASK),             //0x0009
    STYLE_MASK(SS_USERITEM, SS_TYPEMASK),               //0x000A
    STYLE_MASK(SS_SIMPLE, SS_TYPEMASK),                 //0x000B
    STYLE_MASK(SS_LEFTNOWORDWRAP, SS_TYPEMASK),         //0x000C
    STYLE_MASK(SS_OWNERDRAW, SS_TYPEMASK),              //0x000D
    STYLE_MASK(SS_BITMAP, SS_TYPEMASK),                 //0x000E
    STYLE_MASK(SS_ENHMETAFILE, SS_TYPEMASK),            //0x000F
    STYLE_MASK(SS_ETCHEDHORZ, SS_TYPEMASK),             //0x0010
    STYLE_MASK(SS_ETCHEDVERT, SS_TYPEMASK),             //0x0011
    STYLE_MASK(SS_ETCHEDFRAME, SS_TYPEMASK),            //0x0012
    //} STYLE_MASK
    STYLE_SIMPLE(SS_REALSIZECONTROL),                   //0x0040
    STYLE_SIMPLE(SS_NOPREFIX),                          //0x0080

    STYLE_SIMPLE(SS_NOTIFY),                            //0x0100
    STYLE_SIMPLE(SS_CENTERIMAGE),                       //0x0200
    STYLE_SIMPLE(SS_RIGHTJUST),                         //0x0400
    STYLE_SIMPLE(SS_REALSIZEIMAGE),                     //0x0800
    STYLE_SIMPLE(SS_SUNKEN),                            //0x1000
    //{ SS_ELLIPSISMASK
    STYLE_MASK(SS_ENDELLIPSIS, SS_ELLIPSISMASK),        //0x4000
    STYLE_MASK(SS_PATHELLIPSIS, SS_ELLIPSISMASK),       //0x8000
    STYLE_MASK(SS_WORDELLIPSIS, SS_ELLIPSISMASK),       //0xC000
    //} SS_ELLIPSISMASK

    NULL
};

//  Standard Common controls styles
StyleLookupEx CommCtrlList[] =
{
    //{ CCS_TOP_MASK
    STYLE_MASK(CCS_TOP, CCS_TOP_MASK),              //0x0001
    STYLE_MASK(CCS_NOMOVEY, CCS_TOP_MASK),          //0x0002
    STYLE_MASK(CCS_BOTTOM, CCS_TOP_MASK),           //0x0003
    //} CCS_TOP_MASK
    STYLE_SIMPLE(CCS_NORESIZE),                     //0x0004
    STYLE_SIMPLE(CCS_NOPARENTALIGN),                //0x0008
    STYLE_SIMPLE(CCS_ADJUSTABLE),                   //0x0020
    STYLE_SIMPLE(CCS_NODIVIDER),                    //0x0040
    STYLE_SIMPLE(CCS_VERT),                         //0x0080
    STYLE_SIMPLE_DEPENDS(CCS_LEFT, CCS_VERT),       //(CCS_VERT | CCS_TOP)
    STYLE_SIMPLE_DEPENDS(CCS_RIGHT, CCS_VERT),      //(CCS_VERT | CCS_BOTTOM)
    STYLE_SIMPLE_DEPENDS(CCS_NOMOVEX, CCS_VERT),    //(CCS_VERT | CCS_NOMOVEY)

    NULL
};

//  DragList - uses same styles as listview

// Header control (SysHeader32)
StyleLookupEx HeaderStyles[] =
{
    // HDS_HORZ cannot be "not present", as there is no "alternative" defined
    STYLE_SIMPLE(HDS_HORZ),                         //0x0000
    STYLE_SIMPLE(HDS_BUTTONS),                      //0x0002
    STYLE_SIMPLE(HDS_HOTTRACK),                     //0x0004
    STYLE_SIMPLE(HDS_HIDDEN),                       //0x0008
    STYLE_SIMPLE(HDS_DRAGDROP),                     //0x0040
    STYLE_SIMPLE(HDS_FULLDRAG),                     //0x0080
    STYLE_SIMPLE(HDS_FILTERBAR),                    //0x0100
    STYLE_SIMPLE(HDS_FLAT),                         //0x0200
    STYLE_SIMPLE(HDS_CHECKBOXES),                   //0x0400
    STYLE_SIMPLE(HDS_NOSIZING),                     //0x0800
    STYLE_SIMPLE(HDS_OVERFLOW),                     //0x1000

    NULL
};

// Listview (SysListView32)
StyleLookupEx ListViewStyles[] =
{
    //{ LVS_TYPEMASK
    STYLE_MASK(LVS_ICON, LVS_TYPEMASK),             //0x0000
    STYLE_MASK(LVS_REPORT, LVS_TYPEMASK),           //0x0001
    STYLE_MASK(LVS_SMALLICON, LVS_TYPEMASK),        //0x0002
    STYLE_MASK(LVS_LIST, LVS_TYPEMASK),             //0x0003
    //} LVS_TYPEMASK
    STYLE_SIMPLE(LVS_SINGLESEL),                    //0x0004
    STYLE_SIMPLE(LVS_SHOWSELALWAYS),                //0x0008
    STYLE_SIMPLE(LVS_SORTASCENDING),                //0x0010
    STYLE_SIMPLE(LVS_SORTDESCENDING),               //0x0020
    STYLE_SIMPLE(LVS_SHAREIMAGELISTS),              //0x0040
    STYLE_SIMPLE(LVS_NOLABELWRAP),                  //0x0080
    STYLE_SIMPLE(LVS_AUTOARRANGE),                  //0x0100
    STYLE_SIMPLE(LVS_EDITLABELS),                   //0x0200
    STYLE_SIMPLE(LVS_OWNERDATA),                    //0x1000
    STYLE_SIMPLE(LVS_NOSCROLL),                     //0x2000
    //{ LVS_ALIGNMASK
    STYLE_MASK(LVS_ALIGNTOP, LVS_ALIGNMASK),        //0x0000
    STYLE_MASK(LVS_ALIGNLEFT, LVS_ALIGNMASK),       //0x0800
    //} LVS_ALIGNMASK
    STYLE_SIMPLE(LVS_OWNERDRAWFIXED),               //0x0400
    STYLE_SIMPLE(LVS_NOCOLUMNHEADER),               //0x4000
    STYLE_SIMPLE(LVS_NOSORTHEADER),                 //0x8000

    NULL
};

// Toolbar control (ToolbarWindow32)
StyleLookupEx ToolbarStyles[] =
{
    STYLE_SIMPLE(TBSTYLE_TOOLTIPS),         //0x0100
    STYLE_SIMPLE(TBSTYLE_WRAPABLE),         //0x0200
    STYLE_SIMPLE(TBSTYLE_ALTDRAG),          //0x0400
    STYLE_SIMPLE(TBSTYLE_FLAT),             //0x0800
    STYLE_SIMPLE(TBSTYLE_LIST),             //0x1000
    STYLE_SIMPLE(TBSTYLE_CUSTOMERASE),      //0x2000
    STYLE_SIMPLE(TBSTYLE_REGISTERDROP),     //0x4000
    STYLE_SIMPLE(TBSTYLE_TRANSPARENT),      //0x8000

    NULL
};

// Rebar control (RebarControl32)
StyleLookupEx RebarStyles[] =
{
    STYLE_SIMPLE(RBS_TOOLTIPS),             //0x0100
    STYLE_SIMPLE(RBS_VARHEIGHT),            //0x0200
    STYLE_SIMPLE(RBS_BANDBORDERS),          //0x0400
    STYLE_SIMPLE(RBS_FIXEDORDER),           //0x0800
    STYLE_SIMPLE(RBS_REGISTERDROP),         //0x1000
    STYLE_SIMPLE(RBS_AUTOSIZE),             //0x2000
    STYLE_SIMPLE(RBS_VERTICALGRIPPER),      //0x4000
    STYLE_SIMPLE(RBS_DBLCLKTOGGLE),         //0x8000

    NULL
};

// Track Bar control (msctls_trackbar32)
StyleLookupEx TrackbarStyles[] =
{
    STYLE_SIMPLE(TBS_AUTOTICKS),                //0x0001
    STYLE_SIMPLE(TBS_VERT),                     //0x0002
    STYLE_MASK(TBS_HORZ, TBS_VERT),             //0x0000
    STYLE_SIMPLE(TBS_TOP),                      //0x0004
    STYLE_MASK(TBS_BOTTOM, TBS_TOP),            //0x0000
    STYLE_SIMPLE(TBS_LEFT),                     //0x0004
    STYLE_MASK(TBS_RIGHT, TBS_LEFT),            //0x0000
    STYLE_SIMPLE(TBS_BOTH),                     //0x0008
    STYLE_SIMPLE(TBS_NOTICKS),                  //0x0010
    STYLE_SIMPLE(TBS_ENABLESELRANGE),           //0x0020
    STYLE_SIMPLE(TBS_FIXEDLENGTH),              //0x0040
    STYLE_SIMPLE(TBS_NOTHUMB),                  //0x0080
    STYLE_SIMPLE(TBS_TOOLTIPS),                 //0x0100
    STYLE_SIMPLE(TBS_REVERSED),                 //0x0200
    STYLE_SIMPLE(TBS_DOWNISLEFT),               //0x0400
    STYLE_SIMPLE(TBS_NOTIFYBEFOREMOVE),         //0x0800
    STYLE_SIMPLE(TBS_TRANSPARENTBKGND),         //0x1000

    NULL
};

// Treeview (SysTreeView32)
StyleLookupEx TreeViewStyles[] =
{
    STYLE_SIMPLE(TVS_HASBUTTONS),               //0x0001
    STYLE_SIMPLE(TVS_HASLINES),                 //0x0002
    STYLE_SIMPLE(TVS_LINESATROOT),              //0x0004
    STYLE_SIMPLE(TVS_EDITLABELS),               //0x0008
    STYLE_SIMPLE(TVS_DISABLEDRAGDROP),          //0x0010
    STYLE_SIMPLE(TVS_SHOWSELALWAYS),            //0x0020
    STYLE_SIMPLE(TVS_RTLREADING),               //0x0040
    STYLE_SIMPLE(TVS_NOTOOLTIPS),               //0x0080
    STYLE_SIMPLE(TVS_CHECKBOXES),               //0x0100
    STYLE_SIMPLE(TVS_TRACKSELECT),              //0x0200
    STYLE_SIMPLE(TVS_SINGLEEXPAND),             //0x0400
    STYLE_SIMPLE(TVS_INFOTIP),                  //0x0800
    STYLE_SIMPLE(TVS_FULLROWSELECT),            //0x1000
    STYLE_SIMPLE(TVS_NOSCROLL),                 //0x2000
    STYLE_SIMPLE(TVS_NONEVENHEIGHT),            //0x4000
    STYLE_SIMPLE(TVS_NOHSCROLL),                //0x8000

    NULL
};

// Tooltips (tooltips_class32)
StyleLookupEx ToolTipStyles[] =
{
    STYLE_SIMPLE(TTS_ALWAYSTIP),                //0x01
    STYLE_SIMPLE(TTS_NOPREFIX),                 //0x02
    STYLE_SIMPLE(TTS_NOANIMATE),                //0x10
    STYLE_SIMPLE(TTS_NOFADE),                   //0x20
    STYLE_SIMPLE(TTS_BALLOON),                  //0x40
    STYLE_SIMPLE(TTS_CLOSE),                    //0x80
    STYLE_SIMPLE(TTS_USEVISUALSTYLE),           //0x100

    NULL
};

// Statusbar (msctls_statusbar32)
StyleLookupEx StatusBarStyles[] =
{
    STYLE_SIMPLE(SBARS_SIZEGRIP),               //0x0100
    STYLE_SIMPLE(SBARS_TOOLTIPS),               //0x0800

    NULL
};

// Updown control
StyleLookupEx UpDownStyles[] =
{
    STYLE_SIMPLE(UDS_WRAP),                     //0x0001
    STYLE_SIMPLE(UDS_SETBUDDYINT),              //0x0002
    STYLE_SIMPLE(UDS_ALIGNRIGHT),               //0x0004
    STYLE_SIMPLE(UDS_ALIGNLEFT),                //0x0008
    STYLE_SIMPLE(UDS_AUTOBUDDY),                //0x0010
    STYLE_SIMPLE(UDS_ARROWKEYS),                //0x0020
    STYLE_SIMPLE(UDS_HORZ),                     //0x0040
    STYLE_SIMPLE(UDS_NOTHOUSANDS),              //0x0080
    STYLE_SIMPLE(UDS_HOTTRACK),                 //0x0100

    NULL
};

// Progress control (msctls_progress32)
StyleLookupEx ProgressStyles[] =
{
    STYLE_SIMPLE(PBS_SMOOTH),               //0x01
    STYLE_SIMPLE(PBS_VERTICAL),             //0x04

    NULL
};

// Tab control (SysTabControl32)
StyleLookupEx TabStyles[] =
{
    STYLE_SIMPLE(TCS_SCROLLOPPOSITE),                       //0x0001
    STYLE_MASK_DEPENDS(TCS_BOTTOM, 0, 0, TCS_VERTICAL),     //0x0002
    STYLE_SIMPLE_DEPENDS(TCS_RIGHT, TCS_VERTICAL),          //0x0002
    STYLE_SIMPLE(TCS_MULTISELECT),                          //0x0004
    STYLE_SIMPLE(TCS_FLATBUTTONS),                          //0x0008
    STYLE_SIMPLE(TCS_FORCEICONLEFT),                        //0x0010
    STYLE_SIMPLE(TCS_FORCELABELLEFT),                       //0x0020
    STYLE_SIMPLE(TCS_HOTTRACK),                             //0x0040
    STYLE_SIMPLE(TCS_VERTICAL),                             //0x0080
    STYLE_MASK(TCS_TABS, TCS_BUTTONS),                      //0x0000
    STYLE_SIMPLE(TCS_BUTTONS),                              //0x0100
    STYLE_MASK(TCS_SINGLELINE, TCS_MULTILINE),              //0x0000
    STYLE_SIMPLE(TCS_MULTILINE),                            //0x0200
    STYLE_MASK(TCS_RIGHTJUSTIFY, TCS_FIXEDWIDTH),           //0x0000
    STYLE_SIMPLE(TCS_FIXEDWIDTH),                           //0x0400
    STYLE_SIMPLE(TCS_RAGGEDRIGHT),                          //0x0800
    STYLE_SIMPLE(TCS_FOCUSONBUTTONDOWN),                    //0x1000
    STYLE_SIMPLE(TCS_OWNERDRAWFIXED),                       //0x2000
    STYLE_SIMPLE(TCS_TOOLTIPS),                             //0x4000
    STYLE_SIMPLE(TCS_FOCUSNEVER),                           //0x8000

    NULL
};

// Animation control (SysAnimate32)
StyleLookupEx AnimateStyles[] =
{
    STYLE_SIMPLE(ACS_CENTER),                   //0x0001
    STYLE_SIMPLE(ACS_TRANSPARENT),              //0x0002
    STYLE_SIMPLE(ACS_AUTOPLAY),                 //0x0004
    STYLE_SIMPLE(ACS_TIMER),                    //0x0008

    NULL
};

// Month-calendar control (SysMonthCal32)
StyleLookupEx MonthCalStyles[] =
{
    STYLE_SIMPLE(MCS_DAYSTATE),                     //0x0001
    STYLE_SIMPLE(MCS_MULTISELECT),                  //0x0002
    STYLE_SIMPLE(MCS_WEEKNUMBERS),                  //0x0004
    STYLE_SIMPLE(MCS_NOTODAYCIRCLE),                //0x0008
    STYLE_SIMPLE(MCS_NOTODAY),                      //0x0010
    STYLE_SIMPLE(MCS_NOTRAILINGDATES),              //0x0040
    STYLE_SIMPLE(MCS_SHORTDAYSOFWEEK),              //0x0080
    STYLE_SIMPLE(MCS_NOSELCHANGEONNAV),             //0x0100

    NULL
};

// Date-Time picker (SysDateTimePick32)
StyleLookupEx DateTimeStyles[] =
{
    STYLE_SIMPLE(DTS_UPDOWN),                                   //0x0001
    STYLE_SIMPLE(DTS_SHOWNONE),                                 //0x0002
    //{ DTS_FORMAT_MASK
    STYLE_MASK(DTS_SHORTDATEFORMAT, DTS_FORMAT_MASK),           //0x0000
    STYLE_MASK(DTS_LONGDATEFORMAT, DTS_FORMAT_MASK),            //0x0004
    STYLE_MASK(DTS_SHORTDATECENTURYFORMAT, DTS_FORMAT_MASK),    //0x000C
    STYLE_MASK(DTS_TIMEFORMAT, DTS_FORMAT_MASK),                //0x0009
    //} DTS_FORMAT_MASK
    STYLE_SIMPLE(DTS_APPCANPARSE),                              //0x0010
    STYLE_SIMPLE(DTS_RIGHTALIGN),                               //0x0020

    NULL
};

// Pager control (SysPager)
StyleLookupEx PagerStyles[] =
{
    //Pager control
    STYLE_MASK(PGS_VERT, PGS_HORZ),             //0x0000
    STYLE_SIMPLE(PGS_HORZ),                     //0x0001
    STYLE_SIMPLE(PGS_AUTOSCROLL),               //0x0002
    STYLE_SIMPLE(PGS_DRAGNDROP),                //0x0004

    NULL
};

// Extended window styles (for all windows)
StyleLookupEx StyleExList[] =
{
    STYLE_SIMPLE(WS_EX_DLGMODALFRAME),                          //0x00000001L
    STYLE_SIMPLE(WS_EX_NOPARENTNOTIFY),                         //0x00000004L
    STYLE_SIMPLE(WS_EX_TOPMOST),                                //0x00000008L
    STYLE_SIMPLE(WS_EX_ACCEPTFILES),                            //0x00000010L
    STYLE_SIMPLE(WS_EX_TRANSPARENT),                            //0x00000020L
    STYLE_SIMPLE(WS_EX_MDICHILD),                               //0x00000040L
    STYLE_SIMPLE(WS_EX_TOOLWINDOW),                             //0x00000080L
    STYLE_SIMPLE(WS_EX_WINDOWEDGE),                             //0x00000100L
    STYLE_SIMPLE(WS_EX_CLIENTEDGE),                             //0x00000200L
    STYLE_SIMPLE(WS_EX_CONTEXTHELP),                            //0x00000400L
    STYLE_SIMPLE(WS_EX_RIGHT),                                  //0x00001000L
    STYLE_MASK(WS_EX_LEFT, WS_EX_RIGHT),                        //0x00000000L
    STYLE_SIMPLE(WS_EX_RTLREADING),                             //0x00002000L
    STYLE_MASK(WS_EX_LTRREADING, WS_EX_RTLREADING),             //0x00000000L
    STYLE_SIMPLE(WS_EX_LEFTSCROLLBAR),                          //0x00004000L
    STYLE_MASK(WS_EX_RIGHTSCROLLBAR, WS_EX_LEFTSCROLLBAR),      //0x00000000L
    STYLE_SIMPLE(WS_EX_CONTROLPARENT),                          //0x00010000L
    STYLE_SIMPLE(WS_EX_STATICEDGE),                             //0x00020000L
    STYLE_SIMPLE(WS_EX_APPWINDOW),                              //0x00040000L
    STYLE_COMBINATION(WS_EX_OVERLAPPEDWINDOW),                  //(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
    STYLE_COMBINATION(WS_EX_PALETTEWINDOW),                     //(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)
    STYLE_SIMPLE(WS_EX_LAYERED),                                //0x00080000
    STYLE_SIMPLE(WS_EX_NOINHERITLAYOUT),                        //0x00100000
    STYLE_SIMPLE(WS_EX_LAYOUTRTL),                              //0x00400000
    STYLE_SIMPLE(WS_EX_COMPOSITED),                             //0x02000000
    STYLE_SIMPLE(WS_EX_NOACTIVATE),                             //0x08000000

    NULL
};

// ListView extended styles
StyleLookupEx ListViewExStyles[] =
{
    //ListView control styles
    STYLE_SIMPLE(LVS_EX_GRIDLINES),             //0x00000001
    STYLE_SIMPLE(LVS_EX_SUBITEMIMAGES),         //0x00000002
    STYLE_SIMPLE(LVS_EX_CHECKBOXES),            //0x00000004
    STYLE_SIMPLE(LVS_EX_TRACKSELECT),           //0x00000008
    STYLE_SIMPLE(LVS_EX_HEADERDRAGDROP),        //0x00000010
    STYLE_SIMPLE(LVS_EX_FULLROWSELECT),         //0x00000020
    STYLE_SIMPLE(LVS_EX_ONECLICKACTIVATE),      //0x00000040
    STYLE_SIMPLE(LVS_EX_TWOCLICKACTIVATE),      //0x00000080
    STYLE_SIMPLE(LVS_EX_FLATSB),                //0x00000100
    STYLE_SIMPLE(LVS_EX_REGIONAL),              //0x00000200
    STYLE_SIMPLE(LVS_EX_INFOTIP),               //0x00000400
    STYLE_SIMPLE(LVS_EX_UNDERLINEHOT),          //0x00000800
    STYLE_SIMPLE(LVS_EX_UNDERLINECOLD),         //0x00001000
    STYLE_SIMPLE(LVS_EX_MULTIWORKAREAS),        //0x00002000
    STYLE_SIMPLE(LVS_EX_LABELTIP),              //0x00004000
    STYLE_SIMPLE(LVS_EX_BORDERSELECT),          //0x00008000
    STYLE_SIMPLE(LVS_EX_DOUBLEBUFFER),          //0x00010000
    STYLE_SIMPLE(LVS_EX_HIDELABELS),            //0x00020000
    STYLE_SIMPLE(LVS_EX_SINGLEROW),             //0x00040000
    STYLE_SIMPLE(LVS_EX_SNAPTOGRID),            //0x00080000
    STYLE_SIMPLE(LVS_EX_SIMPLESELECT),          //0x00100000
    STYLE_SIMPLE(LVS_EX_JUSTIFYCOLUMNS),        //0x00200000
    STYLE_SIMPLE(LVS_EX_TRANSPARENTBKGND),      //0x00400000
    STYLE_SIMPLE(LVS_EX_TRANSPARENTSHADOWTEXT), //0x00800000
    STYLE_SIMPLE(LVS_EX_AUTOAUTOARRANGE),       //0x01000000
    STYLE_SIMPLE(LVS_EX_HEADERINALLVIEWS),      //0x02000000
    STYLE_SIMPLE(LVS_EX_AUTOCHECKSELECT),       //0x08000000
    STYLE_SIMPLE(LVS_EX_AUTOSIZECOLUMNS),       //0x10000000
    STYLE_SIMPLE(LVS_EX_COLUMNSNAPPOINTS),      //0x40000000
    STYLE_SIMPLE(LVS_EX_COLUMNOVERFLOW),        //0x80000000

    NULL
};

// ComboBoxEx extended styles
StyleLookupEx ComboBoxExStyles[] =
{
    STYLE_SIMPLE(CBES_EX_NOEDITIMAGE),              //0x00000001
    STYLE_SIMPLE(CBES_EX_NOEDITIMAGEINDENT),        //0x00000002
    STYLE_SIMPLE(CBES_EX_PATHWORDBREAKPROC),        //0x00000004
    STYLE_SIMPLE(CBES_EX_NOSIZELIMIT),              //0x00000008
    STYLE_SIMPLE(CBES_EX_CASESENSITIVE),            //0x00000010
    STYLE_SIMPLE(CBES_EX_TEXTENDELLIPSIS),          //0x00000020

    NULL
};

// Tab control extended styles
StyleLookupEx TabCtrlExStyles[] =
{
    STYLE_SIMPLE(TCS_EX_FLATSEPARATORS),            //0x00000001
    STYLE_SIMPLE(TCS_EX_REGISTERDROP),              //0x00000002

    NULL
};

// Toolbar extended styles
StyleLookupEx ToolBarExStyles[] =
{
    STYLE_SIMPLE(TBSTYLE_EX_DRAWDDARROWS),              //0x0001
    STYLE_SIMPLE(TBSTYLE_EX_MIXEDBUTTONS),              //0x0008
    STYLE_SIMPLE(TBSTYLE_EX_HIDECLIPPEDBUTTONS),        //0x0010
    STYLE_SIMPLE(TBSTYLE_EX_DOUBLEBUFFER),              //0x0080

    NULL
};

// Support RichEdit Event masks!!!
StyleLookupEx RichedEventMask[] =
{
    STYLE_SIMPLE(ENM_NONE),                 //0x00000000
    STYLE_SIMPLE(ENM_CHANGE),               //0x00000001
    STYLE_SIMPLE(ENM_UPDATE),               //0x00000002
    STYLE_SIMPLE(ENM_SCROLL),               //0x00000004
    STYLE_SIMPLE(ENM_SCROLLEVENTS),         //0x00000008
    STYLE_SIMPLE(ENM_DRAGDROPDONE),         //0x00000010
    STYLE_SIMPLE(ENM_PARAGRAPHEXPANDED),    //0x00000020
    STYLE_SIMPLE(ENM_PAGECHANGE),           //0x00000040
    STYLE_SIMPLE(ENM_KEYEVENTS),            //0x00010000
    STYLE_SIMPLE(ENM_MOUSEEVENTS),          //0x00020000
    STYLE_SIMPLE(ENM_REQUESTRESIZE),        //0x00040000
    STYLE_SIMPLE(ENM_SELCHANGE),            //0x00080000
    STYLE_SIMPLE(ENM_DROPFILES),            //0x00100000
    STYLE_SIMPLE(ENM_PROTECTED),            //0x00200000
    STYLE_SIMPLE(ENM_CORRECTTEXT),          //0x00400000
    STYLE_SIMPLE(ENM_IMECHANGE),            //0x00800000
    STYLE_SIMPLE(ENM_LANGCHANGE),           //0x01000000
    STYLE_SIMPLE(ENM_OBJECTPOSITIONS),      //0x02000000
    STYLE_SIMPLE(ENM_LINK),                 //0x04000000
    STYLE_SIMPLE(ENM_LOWFIRTF),             //0x08000000

    NULL
};

//
//  Lookup table which matches window classnames to style-lists
//
ClassStyleLookup StandardControls[] =
{
    _T("#32770"),               DialogStyles,       0,
    _T("Button"),               ButtonStyles,       0,
    _T("ComboBox"),             ComboStyles,        0,
    _T("Edit"),                 EditStyles,         0,
    _T("ListBox"),              ListBoxStyles,      0,
    _T("ComboLBox"),            ListBoxStyles,      0,

    _T("RICHEDIT"),             RichedStyles,       0,
    _T("RichEdit20A"),          RichedStyles,       0,
    _T("RichEdit20W"),          RichedStyles,       0,
    _T("RICHEDIT50W"),          RichedStyles,       0,

    _T("Scrollbar"),            ScrollbarStyles,    0,
    _T("Static"),               StaticStyles,       0,

    _T("SysAnimate32"),         AnimateStyles,      0,
    _T("ComboBoxEx"),           ComboStyles,        0,  //(Just a normal combobox)
    _T("SysDateTimePick32"),    DateTimeStyles,     0,
    _T("DragList"),             ListBoxStyles,      0,  //(Just a normal list)
    _T("SysHeader32"),          HeaderStyles,       0,
    //"SysIPAddress32",         IPAddressStyles,    0,  (NO STYLES)
    _T("SysListView32"),        ListViewStyles,     0,
    _T("SysMonthCal32"),        MonthCalStyles,     0,
    _T("SysPager"),             PagerStyles,        0,
    _T("msctls_progress32"),    ProgressStyles,     0,
    _T("RebarWindow32"),        RebarStyles,        0,
    _T("msctls_statusbar32"),   StatusBarStyles,    0,
    //"SysLink",                SysLinkStyles,      0,  (DO IT!)
    _T("SysTabControl32"),      TabStyles,          0,
    _T("ToolbarWindow32"),      ToolbarStyles,      0,
    _T("tooltips_class32"),     ToolTipStyles,      0,
    _T("msctls_trackbar32"),    TrackbarStyles,     0,
    _T("SysTreeView32"),        TreeViewStyles,     0,
    _T("msctls_updown32"),      UpDownStyles,       0,

    NULL
};

// Classes which use the CCS_xxx styles
ClassStyleLookup CustomControls[] =
{
    _T("msctls_statusbar32"),   CommCtrlList,       0,
    _T("RebarWindow32"),        CommCtrlList,       0,
    _T("ToolbarWindow32"),      CommCtrlList,       0,
    _T("SysHeader32"),          CommCtrlList,       0,

    NULL
};

// Classes which have extended window styles
ClassStyleLookup ExtendedControls[] =
{
    _T("SysTabControl32"),      TabCtrlExStyles,    TCM_GETEXTENDEDSTYLE,
    _T("ToolbarWindow32"),      ToolBarExStyles,    TB_GETEXTENDEDSTYLE,
    _T("ComboBox"),             ComboBoxExStyles,   CBEM_GETEXTENDEDSTYLE,
    _T("SysListView32"),        ListViewExStyles,   LVM_GETEXTENDEDLISTVIEWSTYLE,
    _T("RICHEDIT"),             RichedEventMask,    EM_GETEVENTMASK,
    _T("RichEdit20A"),          RichedEventMask,    EM_GETEVENTMASK,
    _T("RichEdit20W"),          RichedEventMask,    EM_GETEVENTMASK,

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
//  Find all the styles that match from the specified list
//
//  StyleList  - list of styles
//  hwndList   - listbox to add styles to
//  dwStyles   - styles for the target window
//  fAllStyles - when true, add all known styles and select those present in the dwStyles value;
//               otherwise, only add the ones that are both applicable and present
//
DWORD EnumStyles(StyleLookupEx *StyleList, HWND hwndList, DWORD dwStyles, BOOL fAllStyles)
{
    // Remember what the dwStyles was before we start modifying it
    DWORD dwOrig = dwStyles;

    int            i, idx;
    BOOL           fPresent;
    StyleLookupEx *pStyle;

    //
    //  Loop through all of the styles that we know about
    //  Check each style against our window's one, to see
    //  if it is set or not
    //
    for (i = 0; StyleList[i].name; i++)
    {
        pStyle = &StyleList[i];

        fPresent = StyleApplicableAndPresent(dwOrig, pStyle);

        // Now add the style.
        if (fPresent || fAllStyles)
        {
            // We've added this style, so remove it to stop it appearing again
            if (fPresent)
                dwStyles &= ~(pStyle->value);

            // Add to list, and set the list's extra item data to the style's data
            idx = (int)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)pStyle->name);
            SendMessage(hwndList, LB_SETITEMDATA, idx, (LPARAM)pStyle);

            if (fAllStyles)
                SendMessage(hwndList, LB_SETSEL, fPresent, idx);
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
    // Empty the list
    SendMessage(hwndStyleList, LB_RESETCONTENT, 0, 0);

    if (!hwndTarget)
        return;

    SendMessage(hwndStyleList, WM_SETREDRAW, FALSE, 0);

    TCHAR szClassName[256];
    StyleLookupEx *StyleList;

    //window class
    GetClassName(hwndTarget, szClassName, ARRAYSIZE(szClassName));

    // Adjust the name for winforms.
    ExtractWindowsFormsInnerClassName(szClassName);

    // enumerate the standard window styles, for any window no
    // matter what class it might be
    DWORD remainingStyles = EnumStyles(WindowStyles, hwndStyleList, dwStyles, fAllStyles);

    // if the window class is one we know about, then see if we
    // can decode any more style bits
    // enumerate the custom control styles
    StyleList = FindStyleList(StandardControls, szClassName, 0);
    if (StyleList != 0)
        // There are cases where specific control styles override the standard window styles (e.g., ES_SELECTIONBAR),
        // so pass the original styles value in
        remainingStyles &= EnumStyles(StyleList, hwndStyleList, dwStyles, fAllStyles);

    // does the window support the CCS_xxx styles (custom control styles)?
    StyleList = FindStyleList(CustomControls, szClassName, 0);
    if (StyleList != 0)
        remainingStyles = EnumStyles(StyleList, hwndStyleList, remainingStyles, fAllStyles);

    // if there are still style bits set in the window style,
    // then there is something that we can't decode. Just display
    // a single HEX entry at the end of the list.
    if (remainingStyles != 0)
    {
        int idx;
        TCHAR ach[10];

        _stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, remainingStyles);
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
    // Empty the list
    SendMessage(hwndExStyleList, LB_RESETCONTENT, 0, 0);

    if (!hwndTarget)
        return;

    SendMessage(hwndExStyleList, WM_SETREDRAW, FALSE, 0);

    TCHAR szClassName[256];
    DWORD dwMessage;
    StyleLookupEx *StyleList;

    //window class
    GetClassName(hwndTarget, szClassName, ARRAYSIZE(szClassName));

    // Adjust the name for winforms.
    ExtractWindowsFormsInnerClassName(szClassName);

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
            // Use SendMessageTimeout to prefent winspy from hanging if the
            // process/thread owning the widow isn't responding.  For example,
            // when the other application is broken into a debugger.
            LRESULT lr;
            DWORD_PTR result;

            lr = SendMessageTimeout(
                   hwndTarget,
                   dwMessage,
                   0, 0,
                   SMTO_BLOCK | SMTO_ERRORONEXIT,
                   250, // 1/4 second
                   &result);

            if (lr)
            {
                dwExStyles = (DWORD)result;
                EnumStyles(StyleList, hwndExStyleList, dwExStyles, fAllStyles);
            }
            else
            {
                // Failed to send the message, we don't have a great place to
                // indicate a failure, appending an error message to the end
                // of the style listbox works.
                int idx = (int)SendMessage(hwndExStyleList, LB_ADDSTRING, 0, (LPARAM)L"<Error: Failed to query custom styles>");
                SendMessage(hwndExStyleList, LB_SETITEMDATA, idx, 0);
            }
        }
    }

    SendMessage(hwndExStyleList, WM_SETREDRAW, TRUE, 0);
}

//
//  Update the Style tab with styles for specified window
//
void SetStyleInfo(HWND hwnd)
{
    TCHAR ach[20];
    DWORD dwStyles = 0;
    DWORD dwExStyles = 0;

    HWND hwndDlg = WinSpyTab[STYLE_TAB].hwnd;
    HWND hwndStyle, hwndStyleEx;

    *ach = 0;

    BOOL fValid = hwnd != NULL;
    if (hwnd && !IsWindow(hwnd))
    {
        fValid = FALSE;
        hwnd = NULL;
        _tcscpy_s(ach, ARRAYSIZE(ach), szInvalidWindow);
    }

    // Display the window style in static label
    if (fValid)
    {
        dwStyles = GetWindowLong(hwnd, GWL_STYLE);
        _stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, dwStyles);
    }
    SetDlgItemText(hwndDlg, IDC_STYLE, ach);

    // Display the extended window style in static label
    if (fValid)
    {
        dwExStyles = GetWindowLong(hwnd, GWL_EXSTYLE);
        _stprintf_s(ach, ARRAYSIZE(ach), szHexFmt, dwExStyles);
    }
    SetDlgItemText(hwndDlg, IDC_STYLEEX, ach);

    // Find handles to standard and extended style lists
    hwndStyle = GetDlgItem(hwndDlg, IDC_LIST1);
    hwndStyleEx = GetDlgItem(hwndDlg, IDC_LIST2);

    // Fill both lists with their styles!
    FillStyleLists(hwnd, hwndStyle, FALSE, dwStyles);
    FillExStyleLists(hwnd, hwndStyleEx, FALSE, dwExStyles, TRUE);
}
