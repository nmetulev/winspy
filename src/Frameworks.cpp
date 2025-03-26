#include "WinSpy.h"

#include "resource.h"
#include "BitmapButton.h"
#include "CaptureWindow.h"
#include "Utils.h"
#include <Psapi.h>
#include <unordered_map>
#include <string>

INT_PTR CALLBACK FrameworksDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    (void)hwnd;
    (void)iMsg;
    (void)wParam;
    (void)lParam;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        return TRUE;
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
    
    // Clear all items from the list
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

    wchar_t buffer[256];
    memset(buffer, 0, sizeof(buffer));
    GetClassNameW(hwnd, buffer, ARRAYSIZE(buffer));

    bool usingChromium = false;
    bool usingDui = false;
    bool usingComCtl = false;

    std::unordered_map<std::wstring, bool*> classMap = {
        {L"Chrome_RenderWidgetHostHWND", &usingChromium},
        {L"Chrome_WidgetWin_1", &usingChromium},
        {L"DirectUIHWND", &usingDui},
        {L"SysListView32", &usingComCtl},
        {L"SysTreeView32", &usingComCtl},
        {L"SysTabControl32", &usingComCtl},
        {L"SysHeader32", &usingComCtl},
        {L"SysAnimate32", &usingComCtl},
        {L"SysDateTimePick32", &usingComCtl},
        {L"SysMonthCal32", &usingComCtl},
        {L"SysIPAddress32", &usingComCtl},
        {L"ToolbarWindow32", &usingComCtl},
        {L"ReBarWindow32", &usingComCtl}
    };

    std::unordered_map<bool*, std::wstring> frameworkMap = {
        {&usingChromium, L"Chromium"},
        {&usingDui, L"DirectUI"},
        {&usingComCtl, L"ComCtl32"}
    };

    auto it = classMap.find(buffer);
    if (it != classMap.end()) {
        *(it->second) = true;
    }

    for (const auto& framework : frameworkMap) {
        if (*(framework.first)) {
            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)framework.second.c_str());
        }
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
            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Electron"));
        }

        // Get list of DLLs loaded
        std::unordered_map<std::wstring, std::wstring> moduleMap = {
            {L"windows.ui.xaml.dll", L"System Xaml (windows.ui.xaml.dll)"},
            {L"presentationcore.dll", L"WPF"},
            {L"presentationcore.ni.dll", L"WPF"},
            {L"webview2loader.dll", L"WebView2"},
            {L"microsoft.reactnative.dll", L"ReactNative"},
            {L"flutter_windows.dll", L"Flutter"},
            {L"libcef.dll", L"CEF"},
            {L"electron_native_auth.node", L"Electron"},
        };

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
                        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)modIt->second.c_str());
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
                                    SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)version);
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
