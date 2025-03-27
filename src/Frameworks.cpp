#include "WinSpy.h"
#include "resource.h"
#include "BitmapButton.h"
#include "CaptureWindow.h"
#include "Utils.h"
#include <Psapi.h>
#include <unordered_map>
#include <string>
#include <CommCtrl.h>
#include <wil/resource.h>
#include <memory>
#include <array>

// Map to store framework-specific icon indices
static std::unordered_map<std::wstring_view, int> frameworkIconMap;

// Helper function to add an item to the ListView
void AddListViewItem(HWND hwndList, std::wstring_view text, std::wstring_view iconKey = L"")
{
    std::wstring_view iconKeyActual = iconKey.empty() ? text : iconKey;
    int iconIndex = frameworkIconMap.count(iconKeyActual) ? frameworkIconMap[iconKeyActual] : frameworkIconMap[L"Unknown"];

    LVITEM lvItem = {};
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvItem.iItem = ListView_GetItemCount(hwndList);
    lvItem.pszText = const_cast<LPWSTR>(text.data());
    lvItem.iImage = iconIndex;
    ListView_InsertItem(hwndList, &lvItem);
}


INT_PTR CALLBACK FrameworksDlgProc(HWND hwnd, UINT iMsg, WPARAM, LPARAM)
{
    static wil::unique_himagelist hImageList;

    switch (iMsg)
    {
    case WM_INITDIALOG:
    {
        HWND hwndList = GetDlgItem(hwnd, IDC_LIST1);
        if (!hwndList) {
            // Log or handle error: ListView control not found
            return FALSE;
        }

        // Set ListView to use full-row select and vertical list mode
        SetWindowLong(hwndList, GWL_STYLE, GetWindowLong(hwndList, GWL_STYLE) | LVS_LIST);
        ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);

        // Map of framework names to image resources
        static const std::unordered_map<std::wstring_view, LPCTSTR> iconMap = {
            {L"CEF", MAKEINTRESOURCE(IDB_FRAMEWORK_CEF)},
            {L"Chromium", MAKEINTRESOURCE(IDB_FRAMEWORK_CHROMIUM)},
            {L"ComCtl32", MAKEINTRESOURCE(IDB_FRAMEWORK_COMCTL32)},
            {L"DirectUI", MAKEINTRESOURCE(IDB_FRAMEWORK_DIRECTUI)},
            {L"Electron", MAKEINTRESOURCE(IDB_FRAMEWORK_ELECTRON)},
            {L"Flutter", MAKEINTRESOURCE(IDB_FRAMEWORK_FLUTTER)},
            {L"React Native", MAKEINTRESOURCE(IDB_FRAMEWORK_REACT_NATIVE)},
            {L"System Xaml", MAKEINTRESOURCE(IDB_FRAMEWORK_WINUI)},
            {L"WinUI", MAKEINTRESOURCE(IDB_FRAMEWORK_WINUI)},
            {L"WPF", MAKEINTRESOURCE(IDB_FRAMEWORK_WPF)},
            {L"WebView2", MAKEINTRESOURCE(IDB_FRAMEWORK_WEBVIEW2)},
            {L"Unknown", IDI_APPLICATION} // Default icon for unmapped frameworks
        };

        // Create an image list for framework icons (32x32 dimensions)
        hImageList.reset(ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, static_cast<int>(iconMap.size()), 1));
        if (!hImageList) {
            OutputDebugString(L"Failed to create image list.\n");
            return FALSE;
        }

        // Load BMP icons dynamically
        for (const auto& [framework, resourceName] : iconMap) {
            wil::unique_hicon hIcon(LoadIcon(nullptr, resourceName));
            if (hIcon) {
                frameworkIconMap.try_emplace(framework, ImageList_AddIcon(hImageList.get(), hIcon.get()));
            } else {
                wil::unique_hbitmap hBitmap(static_cast<HBITMAP>(LoadImage(GetModuleHandle(nullptr), resourceName, IMAGE_BITMAP, 32, 32, LR_LOADTRANSPARENT | LR_CREATEDIBSECTION)));
                if (hBitmap) {
                    int index = ImageList_AddMasked(hImageList.get(), hBitmap.get(), RGB(255, 255, 255)); // Transparency color
                    frameworkIconMap.try_emplace(framework, index);
                } else {
                    OutputDebugString(L"Failed to load bitmap for framework");
                }
            }
        }

        // Associate the image list with the ListView
        ListView_SetImageList(hwndList, hImageList.get(), LVSIL_SMALL);

        return TRUE;
    }
    case WM_DESTROY:
        hImageList.reset();
        break;

    default:
        break;
    }

    return FALSE;
}

void UpdateFrameworksTab(HWND hwnd)
{
    // TODO: I notice this gets called even when the user is mousing over a window,
    // not just when the user releases the mouse button.  Since I expect the frameworks
    // tab may be doing more work in the future, we may want to find a way to
    // only do work when the user releases the mouse button.  AND do expensive work 
    // on a background thread.
    
    HWND hwndDlg = WinSpyTab[FRAMEWORKS_TAB].hwnd;
    HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
    
    // Clear all items from the ListView
    ListView_DeleteAllItems(hwndList);

    std::array<wchar_t, 256> buffer = {};
    GetClassNameW(hwnd, buffer.data(), static_cast<int>(buffer.size()));

    // Wildcard "*" supported, but only at the end of the string.
    static const std::unordered_map<std::wstring_view, std::wstring_view> classMap = {
        {L"Chrome_RenderWidgetHostHWND", L"Chromium"},
        {L"Chrome_WidgetWin_1", L"Chromium"},
        {L"DirectUIHWND", L"DirectUI"},
        {L"SysListView32", L"ComCtl32"},
        {L"SysTreeView32", L"ComCtl32"},
        {L"SysTabControl32", L"ComCtl32"},
        {L"SysHeader32", L"ComCtl32"},
        {L"SysAnimate32", L"ComCtl32"},
        {L"SysDateTimePick32", L"ComCtl32"},
        {L"SysMonthCal32", L"ComCtl32"},
        {L"SysIPAddress32", L"ComCtl32"},
        {L"ToolbarWindow32", L"ComCtl32"},
        {L"ReBarWindow32", L"ComCtl32"},
        {L"Windows.UI.Core.CoreWindow", L"UWP (CoreWindow)"},
        {L"ApplicationFrameInputSinkWindow", L"UWP (ApplicationFrameHost)"},
        {L"AfxFrameOrView*", L"MFC"},
        {L"AfxWnd*", L"MFC" },
        {L"AfxOleControl*", L"MFC" },
        {L"WindowsForms*", L"WinForms"},
        {L"Microsoft.UI.Content.DesktopChildSiteBridge", L"Scene Graph ContentIsland (DesktopChildSiteBridge)"},
        {L"Windows.UI.Composition.DesktopWindowContentBridge", L"System Island (DesktopWindowContentBridge)"},
    };

    static const std::unordered_map<std::wstring_view, std::wstring_view> moduleMap = {
        {L"windows.ui.xaml.dll", L"System Xaml"},
        {L"presentationcore.dll", L"WPF"},
        {L"presentationcore.ni.dll", L"WPF"},
        {L"webview2loader.dll", L"WebView2"},
        {L"microsoft.reactnative.dll", L"React Native"},
        {L"react-native-win32.dll", L"React Native"},
        {L"flutter_windows.dll", L"Flutter"},
        {L"libcef.dll", L"CEF"},
        {L"electron_native_auth.node", L"Electron"},
    };

    for (auto &classPair : classMap) {
        // If classPair.first ends with a *, do a substring match
        if (classPair.first.back() == L'*') {
            // TODO: Inefficient to make a temp string every time through the loop.
            std::wstring className(classPair.first);
            className.pop_back(); // Remove the '*'
            if (wcsstr(buffer, className.c_str()) != nullptr) {
                AddListViewItem(hwndList, classPair.second.c_str());
            }
        }
        else if (classPair.first == buffer) {
            AddListViewItem(hwndList, classPair.second.c_str());
        }
    }


    // Get window's process
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    // Use wil::unique_handle to manage the process handle
    wil::unique_handle hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId));
    if (hProcess) {

        bool usingElectron = false;

        // Process name
        wchar_t processPath[MAX_PATH];
        if (GetModuleFileNameExW(hProcess.get(), nullptr, processPath, 200)) {
            // convert to lowercase for case insensitive comparison
            for (size_t j = 0; processPath[j]; j++) {
                processPath[j] = towlower(processPath[j]);
            }

            // Electron: There is often a file named *.asar near the process
            wchar_t resourcesPath[MAX_PATH];
            wcscpy_s(resourcesPath, processPath);
            wchar_t *lastSlash = wcsrchr(resourcesPath, L'\\');
            if (lastSlash) {
                *lastSlash = L'\0'; // Remove the executable name

                // ELECTRON
                // TOO SLOW.  This was working OK but it does too much work,
                // looking for asar files in the resources path.
                // Also only works in admin mode if the app is packaged, I think.
                // Electron support disabled for now :-(
                // Another possible approach for Electron is looking at the exports
                // of the EXE, I noticed using link /dump /exports that apps like
                // ChatGPT.exe have exports with "electron" in their names.
                // usingElectron = DoesDirContainAsar(resourcesPath);
            }

        }
        else {
            // TODO: Error: L"  Failed to get process name." << std::endl;
        }

        if (usingElectron) {
            AddListViewItem(hwndList, L"Electron");
        }

        // Get list of DLLs loaded
        HMODULE hMods[1024];
        DWORD cbNeeded;
        if (EnumProcessModulesEx(hProcess.get(), hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                wchar_t moduleFullPath[MAX_PATH];
                if (GetModuleFileNameExW(hProcess.get(), hMods[i], moduleFullPath, 200)) {
                    // Trim to just the file name
                    wchar_t *fileName = wcsrchr(moduleFullPath, L'\\');
                    if (fileName) {
                        fileName++; // Move past the backslash
                    } else {
                        fileName = moduleFullPath; // No backslash found, use the whole name
                    }
                    // convert to lowercase for case insensitive comparison
                    for (size_t j = 0; fileName[j]; j++) {
                        fileName[j] = towlower(fileName[j]);
                    }
                    // Convert the key to std::wstring for lookup
                    std::wstring key(fileName);
                    auto modIt = moduleMap.find(key);
                    if (modIt != moduleMap.end()) {
                        AddListViewItem(hwndList, modIt->second);
                    } else if (wcsstr(fileName, L"microsoft.ui.xaml.dll") != nullptr) {
                        // Get the version of the module
                        DWORD versionInfoSize = GetFileVersionInfoSizeW(moduleFullPath, nullptr);
                        if (versionInfoSize > 0) {
                            std::unique_ptr<BYTE[]> versionInfo(new BYTE[versionInfoSize]);
                            if (GetFileVersionInfoW(moduleFullPath, 0, versionInfoSize, versionInfo.get())) {
                                VS_FIXEDFILEINFO *fileInfo;
                                UINT fileInfoSize;
                                if (VerQueryValueW(versionInfo.get(), L"\\", (LPVOID *)&fileInfo, &fileInfoSize)) {
                                    wchar_t version[64];
                                    swprintf_s(version, ARRAYSIZE(version), L"WinUI-%d.%d.%d.%d", HIWORD(fileInfo->dwFileVersionMS), LOWORD(fileInfo->dwFileVersionMS), HIWORD(fileInfo->dwFileVersionLS), LOWORD(fileInfo->dwFileVersionLS));
                                    AddListViewItem(hwndList, version, L"WinUI");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // TODO: Surface this error to the user -- std::wcout << L"  Failed to enumerate modules." << std::endl;
        }
    }
}
