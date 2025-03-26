#include "WinSpy.h"

#include "resource.h"
#include "BitmapButton.h"
#include "CaptureWindow.h"
#include "Utils.h"
#include <Psapi.h>
#include <unordered_map>
#include <string>
#include <CommCtrl.h>

// Helper function to add an item to the ListView
void AddListViewItem(HWND hwndList, const std::wstring& text, int imageIndex)
{
    LVITEM lvItem = {};
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvItem.iItem = ListView_GetItemCount(hwndList);
    lvItem.pszText = const_cast<LPWSTR>(text.c_str());
    lvItem.iImage = imageIndex;
    if (ListView_InsertItem(hwndList, &lvItem) == -1) {
        // Handle error if item insertion fails
    }
}

// Map to store framework-specific icon indices
static std::unordered_map<std::wstring, int> frameworkIconMap;

INT_PTR CALLBACK FrameworksDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    static HIMAGELIST hImageList = nullptr;

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

        // Create an image list for framework icons
        hImageList = ImageList_Create(16, 16, ILC_COLOR32, 1, 10); // Reserve space for 10 icons
        if (hImageList) {
            // Map of framework names to placeholder icons
            static const std::unordered_map<std::wstring, LPCTSTR> iconMap = {
                {L"Chromium", IDI_APPLICATION},
                {L"DirectUI", IDI_INFORMATION},
                {L"ComCtl32", IDI_WARNING},
                {L"WPF", IDI_QUESTION},
                {L"WebView2", IDI_ERROR},
                {L"React Native", IDI_WARNING},
                {L"Flutter", IDI_INFORMATION},
                {L"CEF", IDI_APPLICATION},
                {L"Electron", IDI_QUESTION},
                {L"System Xaml (windows.ui.xaml.dll)", IDI_ERROR}
            };

            // Load icons dynamically
            for (auto it = iconMap.begin(); it != iconMap.end(); ++it) {
                HICON hIcon = LoadIcon(nullptr, it->second);
                if (hIcon) {
                    frameworkIconMap[it->first] = ImageList_AddIcon(hImageList, hIcon);
                    DestroyIcon(hIcon);
                }
            }

            // Associate the image list with the ListView
            ListView_SetImageList(hwndList, hImageList, LVSIL_SMALL);
        } else {
            // Log or handle error: ImageList creation failed
        }

        return TRUE;
    }
    case WM_DESTROY:
        if (hImageList) {
            ImageList_Destroy(hImageList);
            hImageList = nullptr;
        }
        break;

    default:
        // Explicitly mark unused parameters to suppress warnings
        (void)wParam;
        (void)lParam;
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

    wchar_t buffer[256];
    memset(buffer, 0, sizeof(buffer));
    GetClassNameW(hwnd, buffer, ARRAYSIZE(buffer));

    static const std::unordered_map<std::wstring, std::wstring> classMap = {
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
        {L"ReBarWindow32", L"ComCtl32"}
    };

    static const std::unordered_map<std::wstring, std::wstring> moduleMap = {
        {L"windows.ui.xaml.dll", L"System Xaml (windows.ui.xaml.dll)"},
        {L"presentationcore.dll", L"WPF"},
        {L"presentationcore.ni.dll", L"WPF"},
        {L"webview2loader.dll", L"WebView2"},
        {L"microsoft.reactnative.dll", L"React Native"},
        {L"react-native-win32.dll", L"React Native"},
        {L"flutter_windows.dll", L"Flutter"},
        {L"libcef.dll", L"CEF"},
        {L"electron_native_auth.node", L"Electron"},
    };

    auto it = classMap.find(buffer);
    if (it != classMap.end()) {
        int iconIndex = frameworkIconMap.count(it->second) ? frameworkIconMap[it->second] : 0; // Default to placeholder
        AddListViewItem(hwndList, it->second, iconIndex);
    }

    // Get window's process
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {

        bool usingElectron = false;

        // Process name
        wchar_t processPath[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, nullptr, processPath, 200)) {
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
            AddListViewItem(hwndList, L"Electron", 0); // Placeholder icon index
        }

        // Get list of DLLs loaded
        HMODULE hMods[1024];
        DWORD cbNeeded;
        if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                wchar_t moduleFullPath[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, hMods[i], moduleFullPath, 200)) {
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
                    auto modIt = moduleMap.find(fileName);
                    if (modIt != moduleMap.end()) {
                        int iconIndex = frameworkIconMap.count(modIt->second) ? frameworkIconMap[modIt->second] : 0; // Default to placeholder
                        AddListViewItem(hwndList, modIt->second, iconIndex);
                    } else if (wcsstr(fileName, L"microsoft.ui.xaml.dll") != nullptr) {
                        // Get the version of the module
                        DWORD versionInfoSize = GetFileVersionInfoSizeW(moduleFullPath, nullptr);
                        if (versionInfoSize > 0) {
                            BYTE *versionInfo = new BYTE[versionInfoSize];
                            if (GetFileVersionInfoW(moduleFullPath, 0, versionInfoSize, versionInfo)) {
                                VS_FIXEDFILEINFO *fileInfo;
                                UINT fileInfoSize;
                                if (VerQueryValueW(versionInfo, L"\\", (LPVOID *)&fileInfo, &fileInfoSize)) {
                                    wchar_t version[64];
                                    swprintf_s(version, ARRAYSIZE(version), L"WinUI-%d.%d.%d.%d", HIWORD(fileInfo->dwFileVersionMS), LOWORD(fileInfo->dwFileVersionMS), HIWORD(fileInfo->dwFileVersionLS), LOWORD(fileInfo->dwFileVersionLS));
                                    AddListViewItem(hwndList, version, 0); // Placeholder icon index
                                }
                            }
                            delete[] versionInfo;
                        }
                    }
                }
            }
        } else {
            // TODO: Surface this error to the user -- std::wcout << L"  Failed to enumerate modules." << std::endl;
        }
        CloseHandle(hProcess);
    }
}
