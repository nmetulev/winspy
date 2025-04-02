//
//  WinSpyTree.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  Populate the treeview control on the main
//  window with the system window hierarchy.
//

#include "WinSpy.h"

#include <shellapi.h>
#include <malloc.h>

#include "resource.h"
#include "Utils.h"

static HWND       g_hwndTree;
static HIMAGELIST g_hImgList = 0;


//
//  Treeview image indices
//
#define DESKTOP_IMAGE     0         // general images indices
#define INVALID_IMAGE     1         // invalid HWND case
#define WINDOW_IMAGE      2
#define DIALOG_IMAGE      3
#define CHILD_IMAGE       4
#define POPUP_IMAGE       5
#define CONTROL_START     6         // where the control images start
#define NUM_CLASS_BITMAPS 36


//
// TREENODE
//
// Struct used to hold per tree item state.  The TVITEM::lParam for every
// item in the tree is a pointer to one of these.  Different fields are
// relevant for different types of items (process vs. window nodes).
//
// All TREENODE instances are allocated in a global pool (g_TreeNodes).
//

typedef struct
{
    HWND        hwnd;               // Only set for window nodes
    DWORD       dwPID;              // Only set for process nodes
    HTREEITEM   hTreeItem;
}
TREENODE;

TREENODE *g_TreeNodes;
size_t    g_cTreeNodes;
size_t    g_cTreeNodesInUse;


//
//  Use this structure+variables to help us populate the treeview
//
#define MAX_WINDOW_DEPTH 500

typedef struct
{
    HTREEITEM hRoot;
    HWND      hwnd;

}  WinStackType;

typedef struct
{
    DWORD        dwProcessId;
    HTREEITEM    hRoot;             // Main root.

    WinStackType windowStack[MAX_WINDOW_DEPTH];
    int          nWindowZ;          // Current position in the window stack

} WinProc;

WinProc     *g_WinStackList;
int          g_WinStackCount;
HTREEITEM    g_hRoot;

//
//  Define a lookup table, of windowclass to image index
//
typedef struct
{
    PCWSTR szName;          // Class name
    int   index;            // Index into image list
    ATOM  atom;             // (Unused) Might use for fast lookups.

    DWORD  dwAdjustStyles;  // Only valid if one of these styles is set
                            // Default = 0 (don't care)

    DWORD  dwMask;          // Compare

}  ClassImageLookup;

ClassImageLookup ClassImage[] =
{
    L"#32770",               0,  0, 0, 0,
    L"Button",               4,  0, BS_GROUPBOX,         0xF,
    L"Button",               2,  0, BS_CHECKBOX,         0xF,
    L"Button",               2,  0, BS_AUTOCHECKBOX,     0xF,
    L"Button",               2,  0, BS_AUTO3STATE,       0xF,
    L"Button",               2,  0, BS_3STATE,           0xF,
    L"Button",               3,  0, BS_RADIOBUTTON,      0xF,
    L"Button",               3,  0, BS_AUTORADIOBUTTON,  0xF,
    L"Button",               1,  0, 0, 0,    // (default push-button)
    L"ComboBox",             5,  0, 0, 0,
    L"Edit",                 6,  0, 0, 0,
    L"ListBox",              7,  0, 0, 0,

    L"RICHEDIT",             8,  0, 0, 0,
    L"RichEdit20A",          8,  0, 0, 0,
    L"RichEdit20W",          8,  0, 0, 0,
    L"RICHEDIT50W",          8,  0, 0, 0,
    L"RICHEDIT60W",          8,  0, 0, 0,

    L"Scrollbar",            9,  0, SBS_VERT, 0,
    L"Scrollbar",            11, 0, SBS_SIZEBOX | SBS_SIZEGRIP, 0,
    L"Scrollbar",            10, 0, 0, 0,  // (default horizontal)
    L"Static",               12, 0, 0, 0,

    L"SysAnimate32",         13, 0, 0, 0,
    L"SysDateTimePick32",    14, 0, 0, 0,
    L"SysHeader32",          15, 0, 0, 0,
    L"IPAddress",            16, 0, 0, 0,
    L"SysListView32",        17, 0, 0, 0,
    L"SysMonthCal32",        18, 0, 0, 0,
    L"SysPager",             19, 0, 0, 0,
    L"msctls_progress32",    20, 0, 0, 0,
    L"ReBarWindow32",        21, 0, 0, 0,
    L"msctls_statusbar32",   22, 0, 0, 0,
    L"SysLink",              23, 0, 0, 0,
    L"SysTabControl32",      24, 0, 0, 0,
    L"ToolbarWindow32",      25, 0, 0, 0,
    L"tooltips_class32",     26, 0, 0, 0,
    L"msctls_trackbar32",    27, 0, 0, 0,
    L"SysTreeView32",        28, 0, 0, 0,
    L"msctls_updown32",      29, 0, 0, 0,

    L"", 0, 0, 0,
};

//
//  Not used at present (doesn't work!)
//
//  Could be used to perform fast classname lookups, if we
//  precalculate all the class atoms, then all we need to
//  do is find the class-atom for a window, then match it in
//  our lookup table above (instead of doing slow string-searches).
//
void InitAtomList()
{
    int     i;
    ATOM    atom;

    INITCOMMONCONTROLSEX ice;

    ice.dwSize = sizeof(ice);
    ice.dwICC = ICC_COOL_CLASSES;//-1;  //all classes

    i = InitCommonControlsEx(&ice);

    for (i = 0; ClassImage[i].szName[0] != 0; i++)
    {
        atom = GlobalFindAtom(ClassImage[i].szName);

        ClassImage[i].atom = atom;
    }
}

//
//  Find the image index (in TreeView imagelist), given a
//  window classname. dwStyle lets us differentiate further
//  when we find a match.
//
int IconFromClassName(PCWSTR pszName, DWORD dwStyle)
{
    int i = 0;
    WCHAR szCopy[200];

    if (wcscmp(pszName, L"#32769") == 0)
    {
        return DESKTOP_IMAGE;
    }

    if (IsWindowsFormsClassName(pszName))
    {
        wcscpy_s(szCopy, ARRAYSIZE(szCopy), pszName);
        ExtractWindowsFormsInnerClassName(szCopy);
        pszName = szCopy;
    }

    while (ClassImage[i].szName[0] != L'\0')
    {
        if (lstrcmpi(ClassImage[i].szName, pszName) == 0)
        {
            DWORD dwMask = ClassImage[i].dwMask;

            if (ClassImage[i].dwAdjustStyles != 0)
            {
                if (dwMask != 0)
                {
                    if (ClassImage[i].dwAdjustStyles == (dwStyle & dwMask))
                        return  (ClassImage[i].index + CONTROL_START);
                }
                else
                {
                    if (ClassImage[i].dwAdjustStyles & dwStyle)
                        return  (ClassImage[i].index + CONTROL_START);
                }

            }

            if (ClassImage[i].dwAdjustStyles == 0)
                return  (ClassImage[i].index + CONTROL_START);
        }

        i++;
    }

    return -1;
}

#define MAX_VERBOSE_LEN 22
#define MAX_CLASS_LEN   40
#define MAX_WINTEXT_LEN 200

#define MIN_FORMAT_LEN  (32 + MAX_VERBOSE_LEN + MAX_CLASS_LEN + MAX_WINTEXT_LEN)

//
// Computes the treeview item text and icon index for the specified window.
//
// Return value is the image index.
// szTotal must be MIN_FORMAT_LEN characters
//
int CalcNodeTextAndIcon(HWND hwnd, BOOL fIsVisible, DWORD dwStyle, WCHAR szTotal[], int cchTotal)
{
    //ASSERT(cchTotal >= MIN_FORMAT_LEN);
    static WCHAR szClass[MAX_CLASS_LEN + MAX_VERBOSE_LEN];
    int iImage;
    WCHAR *pszCaption;

    DWORD dwCloaked = 0;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &dwCloaked, sizeof(dwCloaked));

    //
    // Window handle in hex format
    //
    if (g_opts.uTreeInclude & WINLIST_INCLUDE_HANDLE)
    {
        swprintf_s(szTotal, cchTotal, L"%08X  ", (UINT)(UINT_PTR)hwnd);
    }
    else
    {
        wcscpy_s(szTotal, cchTotal, L"");
    }

    //
    // Window class name
    //
    GetClassName(hwnd, szClass, MAX_CLASS_LEN);

    dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    iImage = IconFromClassName(szClass, dwStyle);

    if (g_opts.uTreeInclude & WINLIST_INCLUDE_CLASS)
    {
        VerboseClassName(szClass, ARRAYSIZE(szClass), (WORD)GetClassLong(hwnd, GCW_ATOM));

        if (g_opts.fClassThenText)
        {
            wcscat_s(szTotal, cchTotal, szClass);
            wcscat_s(szTotal, cchTotal, L"  ");
        }
    }
    else
    {
        szClass[0] = L'\0';
    }

    wcscat_s(szTotal, cchTotal, L"\"");

    size_t len = wcslen(szTotal);
    pszCaption = szTotal + len;

    size_t cchCaption = min(MAX_WINTEXT_LEN, cchTotal - len);
    // Window title, enclosed in quotes
    if (!SendMessageTimeout(
        hwnd,
        WM_GETTEXT,
        cchCaption,
        (LPARAM)pszCaption,
        SMTO_ABORTIFHUNG, 100, NULL))
    {
        GetWindowText(hwnd, pszCaption, (int)cchCaption);
    }

    // WM_GETTEXT does not guarantee null termination.

    pszCaption[cchCaption - 1] = '\0';

    // If the caption is empty, then remove the leading quote.
    // Otherwise, add the closing quote.

    if (*pszCaption != '\0')
    {
        wcscat_s(szTotal, cchTotal, L"\"");
    }
    else
    {
        *(pszCaption - 1) = '\0';
    }

    if (!g_opts.fClassThenText)
    {
        wcscat_s(szTotal, cchTotal, L"  ");
        wcscat_s(szTotal, cchTotal, szClass);
    }

    // Add cloaked annotation to windows that have been explicitly cloaked.
    if (dwCloaked == DWM_CLOAKED_APP)
    {
        wcscat_s(szTotal, cchTotal, L" [app cloaked]");
    }
    else if (dwCloaked == DWM_CLOAKED_SHELL)
    {
        wcscat_s(szTotal, cchTotal, L" [cloaked]");
    }

    // Pick default images, if we didn't already pick a class specific one.

    if (iImage == -1)
    {
        if (dwStyle & WS_CHILD)
        {
            // child windows (edit boxes, list boxes etc)
            iImage = CHILD_IMAGE;
        }
        else if ((dwStyle & WS_POPUPWINDOW) == WS_POPUPWINDOW)
        {
            // dialog boxes
            iImage = DIALOG_IMAGE;
        }
        else if (dwStyle & WS_POPUP)
        {
            // popup windows (tooltips etc)
            iImage = POPUP_IMAGE;
        }
        else
        {
            // anything else must be a top-level window
            iImage = WINDOW_IMAGE;
        }
    }

    // Adjust icon index for hidden/cloaked windows.
    // The imagelist starts with NUM_CLASS_BITMAPS images to be used for
    // visible windows, followed by an alternate set to be used for hidden
    // windows, then a third set for cloaked windows.

    if (g_opts.fShowDimmed)
    {
        if (dwCloaked != 0)
        {
            iImage += (2 * NUM_CLASS_BITMAPS);
        }
        else if (!fIsVisible)
        {
            iImage += NUM_CLASS_BITMAPS;
        }
    }

    return iImage;
}

//
// Returns a clean/empty TREENODE struct to be used for a newly inserted
// treeview item.
//
// Note that the callers do not need to explicitly manage the lifetime of
// these objects.  They are allocated out of a simple arena allocator.
// When the treeview contents are reset, all outstanding TREENODE instances
// are implicitly freed by virtue of g_cTreeNodesInUse being reset back to
// zero.  The array slots will then be recycled as the tree is repopulated.
//
ptrdiff_t AllocateTreeNode()
{
    // Grow the array if it is full.

    if (g_cTreeNodesInUse == g_cTreeNodes)
    {
        size_t    cNew  = g_cTreeNodes + 1000;
        size_t    cbNew = cNew * sizeof(TREENODE);
        TREENODE *rgNew = (TREENODE *)realloc(g_TreeNodes, cbNew);

        if (!rgNew)
        {
            return -1;
        }

        g_TreeNodes  = rgNew;

        g_cTreeNodes = cNew;
    }

    // Hand out the next item.

    TREENODE *pNode = &g_TreeNodes[g_cTreeNodesInUse];

    ZeroMemory(pNode, sizeof(*pNode));

    return g_cTreeNodesInUse++;
}

//
//
//
WinProc *GetProcessWindowStack(HWND hwndTree, HWND hwnd)
{
    DWORD           pid;
    int             i;
    TVINSERTSTRUCT  tv;
    WCHAR           ach[MIN_FORMAT_LEN];
    WCHAR           name[100] = L"";
    WCHAR           path[MAX_PATH] = L"";
    SHFILEINFO      shfi = { 0 };

    GetWindowThreadProcessId(hwnd, &pid);

    //
    // look for an existing process/window stack:
    //
    for (i = 0; i < g_WinStackCount; i++)
    {
        if (g_WinStackList[i].dwProcessId == pid)
            return &g_WinStackList[i];
    }

    //
    // couldn't find one - build a new one instead
    //
    GetProcessNameByPid(pid, name, 100, path, MAX_PATH);
    swprintf_s(ach, ARRAYSIZE(ach), L"%s  (%u)", name, pid);

    TREENODE *pNode = NULL;
    ptrdiff_t nodeIndex = AllocateTreeNode();

    if (nodeIndex >= 0)
    {
        pNode = &g_TreeNodes[nodeIndex];
        pNode->dwPID = pid;
    }
    else
    {
        return NULL;
    }

    // Add the root item
    tv.hParent = g_hRoot;
    tv.hInsertAfter = TVI_LAST;
    tv.item.mask = TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tv.item.state = 0;//TVIS_EXPANDED;
    tv.item.stateMask = 0;//TVIS_EXPANDED;
    tv.item.pszText = ach;
    tv.item.cchTextMax = ARRAYSIZE(ach);
    tv.item.lParam = (LPARAM)nodeIndex;

    if (SHGetFileInfo(path, 0, &shfi, sizeof(shfi), SHGFI_SMALLICON | SHGFI_ICON))
    {
        tv.item.iImage = ImageList_AddIcon(g_hImgList, shfi.hIcon);
        tv.item.iSelectedImage = tv.item.iImage;
    }
    else
    {
        tv.item.iImage = WINDOW_IMAGE;
        tv.item.iSelectedImage = WINDOW_IMAGE;
    }

    pNode->hTreeItem = TreeView_InsertItem(hwndTree, &tv);

    g_WinStackList[g_WinStackCount].hRoot = pNode->hTreeItem;
    g_WinStackList[g_WinStackCount].dwProcessId = pid;
    g_WinStackList[g_WinStackCount].nWindowZ = 1;
    g_WinStackList[g_WinStackCount].windowStack[0].hRoot = pNode->hTreeItem;
    g_WinStackList[g_WinStackCount].windowStack[0].hwnd = 0;

    return &g_WinStackList[g_WinStackCount++];
}

//
// Callback function which is called once for every window in
// the system. We have to work out whereabouts in the treeview
// to insert each window
//
BOOL CALLBACK AllWindowProc(HWND hwnd, LPARAM lParam)
{
    HWND hwndTree = (HWND)lParam;
    BOOL fIsVisible = IsWindowVisible(hwnd);

    // Ignore it if it is hidden and we are omitting hidden windows from the list.
    if (!fIsVisible && !g_opts.fShowHiddenInList)
        return TRUE;

    static WCHAR szTotal[MIN_FORMAT_LEN];

    int i;

    // Style is used to decide which bitmap to display in the tree
    UINT uStyle = GetWindowLong(hwnd, GWL_STYLE);

    // Need to know the current window's parent, so we know
    // where to insert this window
    HWND  hwndParent = GetRealParent(hwnd);

    // Keep track of the last window to be inserted, so
    // we know the z-order of the current window
    static HTREEITEM hTreeLast;
    static HWND      hwndLast;

    TVINSERTSTRUCT tv;
    TREENODE *pNode = NULL;
    ptrdiff_t nodeIndex = AllocateTreeNode();

    if (nodeIndex >= 0)
    {
        pNode = &g_TreeNodes[nodeIndex];
        pNode->hwnd = hwnd;
    }
    else
    {
        return FALSE;
    }


    //
    //
    //
    WinProc *winProc = GetProcessWindowStack(hwndTree, hwnd);
    WinStackType *WindowStack = winProc->windowStack;

    // Prepare the TVINSERTSTRUCT object
    ZeroMemory(&tv, sizeof(tv));
    tv.hParent = winProc->hRoot;
    tv.hInsertAfter = TVI_LAST;
    tv.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tv.item.pszText = szTotal;
    tv.item.cchTextMax = ARRAYSIZE(szTotal);
    tv.item.lParam = (LPARAM)nodeIndex;

    tv.item.iImage = CalcNodeTextAndIcon(hwnd, fIsVisible, uStyle, szTotal, ARRAYSIZE(szTotal));

    //
    // Insertion position depends on type of window.
    //
    if (uStyle & WS_CHILD)
    {
        // child windows (edit boxes, list boxes etc)
        tv.hInsertAfter = TVI_LAST;
    }
    else if ((uStyle & WS_POPUPWINDOW) == WS_POPUPWINDOW)
    {
        // dialog boxes
        tv.hInsertAfter = TVI_FIRST;
    }
    else if (uStyle & WS_POPUP)
    {
        // popup windows (tooltips etc)
        tv.hInsertAfter = TVI_LAST;
    }
    else
    {
        // anything else must be a top-level window
        tv.hInsertAfter = TVI_FIRST;
    }

    //set the selected bitmap to be the same
    tv.item.iSelectedImage = tv.item.iImage;

    //
    // Decide where to place this item
    //
    //  If this window is in a different Z-order than the last one, then
    //  we need to either start a sub-hierarchy (if it is a child),
    //  or return back up the existing hierarchy.
    //
    if (winProc->nWindowZ > 0 && hwndParent != WindowStack[winProc->nWindowZ - 1].hwnd)
    {
        //we have another child window
        if (hwndParent == hwndLast)
        {
            //make a new parent stack entry
            WindowStack[winProc->nWindowZ].hRoot = hTreeLast;
            WindowStack[winProc->nWindowZ].hwnd = hwndParent;

            if (winProc->nWindowZ < MAX_WINDOW_DEPTH - 1)
                winProc->nWindowZ++;

            tv.hParent = hTreeLast;
        }
        //moving back?????
        else
        {
            //search for this window's parent in the stack so
            //we know where to insert this window under
            for (i = 0; i < winProc->nWindowZ; i++)
            {
                if (WindowStack[i].hwnd == hwndParent)
                {
                    winProc->nWindowZ = i + 1;
                    tv.hParent = WindowStack[i].hRoot;
                }
            }
        }
    }
    // otherwise, this window is a sibling to the last one, so just append
    // it to the treeview, in the same "z-order"
    else
    {
        tv.hParent = WindowStack[winProc->nWindowZ - 1].hRoot;
    }

    // Finally add the node
    pNode->hTreeItem = TreeView_InsertItem(hwndTree, &tv);

    hTreeLast = pNode->hTreeItem;
    hwndLast = hwnd;

    return TRUE;
}

//
//  Populate the treeview control by using EnumChildWindows,
//  starting from the desktop window
//  HWND - handle to the dialog containing the tree
//
void FillGlobalWindowTree(HWND hwndTree)
{
    HWND hwndDesktop = GetDesktopWindow();

    // hwndDesktop = FindWindowEx(HWND_MESSAGE, NULL, NULL, NULL);
    // hwndDesktop = GetRealParent(hwndDesktop);

    if (g_opts.fShowDesktopRoot)
    {
        TVINSERTSTRUCT tv;
        WCHAR ach[MIN_FORMAT_LEN];

        CalcNodeTextAndIcon(hwndDesktop, TRUE, 0, ach, ARRAYSIZE(ach));

        TREENODE *pNode = NULL;
        ptrdiff_t nodeIndex = AllocateTreeNode();

        if (nodeIndex >= 0)
        {
            pNode = &g_TreeNodes[nodeIndex];
            pNode->hwnd = hwndDesktop;
        }
        else
        {
            g_hRoot = TVI_ROOT;
            return;
        }


        //Add the root item
        tv.hParent = TVI_ROOT;
        tv.hInsertAfter = TVI_LAST;
        tv.item.mask = TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        tv.item.state = TVIS_EXPANDED;
        tv.item.stateMask = TVIS_EXPANDED;
        tv.item.pszText = ach;
        tv.item.cchTextMax = ARRAYSIZE(ach);
        tv.item.iImage = DESKTOP_IMAGE;
        tv.item.iSelectedImage = DESKTOP_IMAGE;
        tv.item.lParam = (LPARAM)nodeIndex;

        pNode->hTreeItem = TreeView_InsertItem(hwndTree, &tv);
        g_hRoot = pNode->hTreeItem;
    }
    else
    {
        g_hRoot = TVI_ROOT;
    }

    // EnumChildWindows does the hard work for us

    EnumChildWindows(hwndDesktop, AllWindowProc, (LPARAM)hwndTree);
}

//
//  Initialize the TreeView resource
//
void WindowTree_Initialize(HWND hwndTree)
{
    g_hwndTree = hwndTree;

    // This tells the treeview to scale the IMAGELIST up for DPI.
    SendMessage(hwndTree, CCM_DPISCALE, 1, 0);

    HBITMAP hBitmap;
    TCITEM  tcitem;
    HWND    hwndTab;

    //only need to create the image list once.
    if (g_hImgList == 0)
    {
        // Create an empty image list
        g_hImgList = ImageList_Create(16, 16, ILC_COLOR32 /*ILC_COLORDDB*/ | ILC_MASK, NUM_CLASS_BITMAPS, 8);

        // Load our bitmap and add it to the image list
        hBitmap = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_WINDOW_VISIBLE));
        ImageList_AddMasked(g_hImgList, hBitmap, RGB(255, 0, 255));
        DeleteObject(hBitmap);

        hBitmap = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_WINDOW_INVISIBLE));
        ImageList_AddMasked(g_hImgList, hBitmap, RGB(255, 0, 255));
        DeleteObject(hBitmap);

        hBitmap = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_WINDOW_CLOAKED));
        ImageList_AddMasked(g_hImgList, hBitmap, RGB(255, 0, 255));
        DeleteObject(hBitmap);

        // Assign the image list to the treeview control
        TreeView_SetImageList(hwndTree, g_hImgList, TVSIL_NORMAL);
    }

    //add an item to the tab control
    ZeroMemory(&tcitem, sizeof(tcitem));
    tcitem.mask = TCIF_TEXT;
    tcitem.pszText = L"All Windows by Process";

    hwndTab = GetDlgItem(GetParent(hwndTree), IDC_TAB2);
    SendMessage(hwndTab, TCM_INSERTITEM, 0, (LPARAM)&tcitem);

    //subclass the tab control to remove flicker whilst it is resized
    RemoveTabCtrlFlicker(hwndTab);

    //InitAtomList();
}

//
//  Clean up TreeView resources
//
void WindowTree_Destroy()
{
    TreeView_SetImageList(g_hwndTree, 0, TVSIL_NORMAL);
    ImageList_Destroy(g_hImgList);
}

//
//  Find the specified window in the TreeView.
//
//  Note that there is no need to interrogate state from the treeview.
//  We can simply traverse the list of all TREENODEs to perform the HWND
//  to HTREEITEM mapping.
//

HTREEITEM FindTreeItemByHwnd(HWND hwnd)
{
    if (hwnd)
    {
        for (size_t i = 0; i < g_cTreeNodesInUse; i++)
        {
            if (g_TreeNodes[i].hwnd == hwnd)
            {
                return g_TreeNodes[i].hTreeItem;
            }
        }
    }

    return NULL;
}

//
//  Update the TreeView with current window list
//
void WindowTree_Refresh(HWND hwndToSelect, BOOL fSetFocus)
{
    HWND  hwndTree = g_hwndTree;
    DWORD dwStyle;

    if (!g_WinStackList)
    {
        g_WinStackList = (WinProc*)malloc(1000 * sizeof(WinProc));
    }

    g_WinStackCount = 0;
    g_cTreeNodesInUse = 0;

    EnableWindow(hwndTree, TRUE);

    dwStyle = GetWindowLong(hwndTree, GWL_STYLE);

    // We need to hide the treeview temporarily (turn OFF WS_VISIBLE)
    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);
    SetWindowLong(hwndTree, GWL_STYLE, dwStyle & ~WS_VISIBLE);

    TreeView_DeleteAllItems(hwndTree);

    FillGlobalWindowTree(hwndTree);


    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);
    dwStyle = GetWindowLong(hwndTree, GWL_STYLE);
    SetWindowLong(hwndTree, GWL_STYLE, dwStyle | WS_VISIBLE);


    SetWindowPos(hwndTree, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);

    InvalidateRect(hwndTree, 0, TRUE);

    if (hwndToSelect)
    {
        HTREEITEM hti = FindTreeItemByHwnd(hwndToSelect);

        if (hti)
        {
            SendMessage(hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)hti);
            SendMessage(hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hti);

            if (fSetFocus)
            {
                SetFocus(hwndTree);
            }
        }
    }
}


void WindowTree_OnRightClick(NMHDR *pnm)
{
    TVHITTESTINFO hti;
    TVITEM        tvi;

    UINT   uCmd;
    HMENU  hMenu, hPopup;
    POINT  pt;
    HWND   hwndTree   = pnm->hwndFrom;
    HWND   hwndDialog = GetParent(hwndTree);

    // Find out where in the TreeView the mouse has been clicked
    GetCursorPos(&pt);

    hti.pt = pt;
    ScreenToClient(hwndTree, &hti.pt);

    // Find item which has been right-clicked on
    if (TreeView_HitTest(hwndTree, &hti) &&
        (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)))
    {
        // Now get the window handle, which is stored in the lParam
        // portion of the TVITEM structure.
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_HANDLE | TVIF_PARAM;
        tvi.hItem = hti.hItem;

        TreeView_GetItem(hwndTree, &tvi);

        ptrdiff_t nodeIndex = (ptrdiff_t)tvi.lParam;
        TREENODE *pNode = &g_TreeNodes[nodeIndex];

        if (pNode->hwnd)
        {
            hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU_WINDOW_INTREE));
            hPopup = GetSubMenu(hMenu, 0);

            WinSpy_SetupPopupMenu(hPopup, pNode->hwnd);

            // Show the menu
            uCmd = TrackPopupMenu(hPopup, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwndDialog, 0);

            // Act accordingly
            if (uCmd)
            {
                WinSpy_PopupCommandHandler(hwndDialog, uCmd, pNode->hwnd);

                // If this window is the primary window (aka g_hCurWnd) then refresh
                // the active tab.  This will cause, for example, the visible/hidden
                // status shown on the General tab to update if the user toggled
                // the window visibility.  Note that this will also cause the
                // treeview node to refresh.
                //
                // Otherwise, if this isn't the primary window, then just refresh
                // the treeview node.

                if (pNode->hwnd == g_hCurWnd)
                {
                    UpdateActiveTab();
                }
                else
                {
                    WindowTree_RefreshWindowNode(pNode->hwnd);
                }
            }

            DestroyMenu(hMenu);
        }
        else
        {
            ShowProcessContextMenu(g_hwndTree, pt.x, pt.y, FALSE, pNode->hwnd, pNode->dwPID);
        }
    }
}


void WindowTree_OnSelectionChanged(NMHDR *pnm)
{
    NMTREEVIEW *pnmtv = (NMTREEVIEW *)pnm;
    TVITEM      item;

    //Find the window handle stored in the TreeView item's lParam
    ZeroMemory(&item, sizeof(item));

    item.mask = TVIF_HANDLE | TVIF_PARAM;
    item.hItem = pnmtv->itemNew.hItem;

    // Get TVITEM structure
    TreeView_GetItem(pnm->hwndFrom, &item);

    ptrdiff_t nodeIndex = (ptrdiff_t)item.lParam;
    TREENODE *pNode = &g_TreeNodes[nodeIndex];

    g_dwSelectedPID = pNode->dwPID;

    DisplayWindowInfo(pNode->hwnd);
}


void WindowTree_Locate(HWND hwnd)
{
    HTREEITEM hti = FindTreeItemByHwnd(hwnd);

    if (!hti)
    {
        WindowTree_Refresh(NULL, FALSE);
        hti = FindTreeItemByHwnd(hwnd);
    }

    if (hti)
    {
        SendMessage(g_hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)hti);
        SendMessage(g_hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hti);

        SetFocus(g_hwndTree);
    }
}


HWND WindowTree_GetSelectedWindow()
{
    HTREEITEM hti = TreeView_GetSelection(g_hwndTree);
    HWND hwnd = NULL;

    if (hti)
    {
        TVITEM item;

        ZeroMemory(&item, sizeof(item));

        item.mask = TVIF_PARAM | TVIF_HANDLE;
        item.hItem = hti;

        TreeView_GetItem(g_hwndTree, &item);

        ptrdiff_t nodeIndex = (ptrdiff_t)item.lParam;
        TREENODE *pNode = &g_TreeNodes[nodeIndex];
        hwnd = pNode->hwnd;
    }

    return hwnd;
}


//
// Refreshes the text and icon of the treeview node that correpondes to the
// specified window (if one exists).
//

void WindowTree_RefreshWindowNode(HWND hwnd)
{
    // The tree is manually refreshed, so it can be the case that there is
    // no node in the tree for the live window.

    HTREEITEM hti = FindTreeItemByHwnd(hwnd);

    if (!hti)
    {
        return;
    }

    WCHAR szOld[MIN_FORMAT_LEN];
    WCHAR szNew[MIN_FORMAT_LEN];
    int   iImage;

    // Fetch existing text/icon.

    TVITEM item;
    ZeroMemory(&item, sizeof(item));

    item.mask       = TVIF_HANDLE | TVIF_IMAGE | TVIF_TEXT;
    item.hItem      = hti;
    item.pszText    = szOld;
    item.cchTextMax = ARRAYSIZE(szOld);

    if (!TreeView_GetItem(g_hwndTree, &item))
    {
        return;
    }

    // Compute new text/icon.
    //
    // If the widow is no longer valid then replace the icon with a red X
    // icon to indicate that state, but leave the text alone so that you can
    // tell what it had been.

    if (!IsWindow(hwnd))
    {
        iImage = INVALID_IMAGE;
        wcscpy_s(szNew, ARRAYSIZE(szNew), szOld);
    }
    else
    {
        BOOL fIsVisible = IsWindowVisible(hwnd);
        UINT dwStyle = GetWindowLong(hwnd, GWL_STYLE);

        iImage = CalcNodeTextAndIcon(hwnd, fIsVisible, dwStyle, szNew, ARRAYSIZE(szNew));
    }

    // If they are different, then update.

    if (iImage != item.iImage || (wcscmp(szOld, szNew) != 0))
    {
        item.mask           = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
        item.hItem          = hti;
        item.iImage         = iImage;
        item.iSelectedImage = iImage;
        item.pszText        = szNew;
        item.cchTextMax     = ARRAYSIZE(szNew);

        TreeView_SetItem(g_hwndTree, &item);
    }
}
