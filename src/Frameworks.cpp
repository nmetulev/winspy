#include "WinSpy.h"

#include "resource.h"
#include "BitmapButton.h"
#include "CaptureWindow.h"
#include "Utils.h"
#include <Psapi.h>

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

    if (wcsstr(buffer, L"Chrome_RenderWidgetHostHWND") != nullptr) {
        usingChromium = true;
    }
    else if (wcsstr(buffer, L"Chrome_WidgetWin_1") != nullptr) {
        usingChromium = true;
    }
    else if (wcscmp(buffer, L"DirectUIHWND") == 0) {
        usingDui = true;
    }

    if (usingChromium) {
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Chromium"));
    }
    if (usingDui) {
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("DirectUI"));
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
        HMODULE hMods[1024];
        DWORD cbNeeded;
        if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
            
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                wchar_t moduleName[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, hMods[i], moduleName, 200)) {
                    // convert to lowercase for case insensitive comparison
                    for (size_t j = 0; moduleName[j]; j++) {
                        moduleName[j] = towlower(moduleName[j]);
                    }
                    if (wcsstr(moduleName, L"windows.ui.xaml.dll") != nullptr) {
                        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("System Xaml (windows.ui.xaml.dll)"));
                    }
                    else if (wcsstr(moduleName, L"presentationcore.dll") != nullptr || wcsstr(moduleName, L"presentationcore.ni.dll") != nullptr) {
                        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("WPF"));
                    }
                    else if (wcsstr(moduleName, L"microsoft.ui.xaml.dll") != nullptr) {
                        // Get the version of the module
                        DWORD versionInfoSize = GetFileVersionInfoSizeW(moduleName, nullptr);
                        if (versionInfoSize > 0) {
                            BYTE *versionInfo = new BYTE[versionInfoSize];
                            if (GetFileVersionInfoW(moduleName, 0, versionInfoSize, versionInfo)) {
                                VS_FIXEDFILEINFO *fileInfo;
                                UINT fileInfoSize;
                                if (VerQueryValueW(versionInfo, L"\\", (LPVOID *)&fileInfo, &fileInfoSize)) {
                                    //std::wcout << L"WinUI-" << HIWORD(fileInfo->dwFileVersionMS) << L"." << LOWORD(fileInfo->dwFileVersionMS) << L"." << HIWORD(fileInfo->dwFileVersionLS) << L"." << LOWORD(fileInfo->dwFileVersionLS) << L" ";
                                    wchar_t version[64];
                                    swprintf_s(version, ARRAYSIZE(version), L"WinUI-%d.%d.%d.%d", HIWORD(fileInfo->dwFileVersionMS), LOWORD(fileInfo->dwFileVersionMS), HIWORD(fileInfo->dwFileVersionLS), LOWORD(fileInfo->dwFileVersionLS));
                                    SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)version);
                                }
                            }
                            delete[] versionInfo;
                        }
                    }
                    else if (wcsstr(moduleName, L"webview2loader.dll") != nullptr) {
                        SendDlgItemMessage(hwndDlg, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)TEXT("WebView2"));
                    }
                    else if (wcsstr(moduleName, L"microsoft.reactnative.dll") != nullptr) {
                        SendDlgItemMessage(hwndDlg, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)TEXT("ReactNative"));
                    }

                }
            }
        }
        else {
            // TODO: Surface this error to the user -- std::wcout << L"  Failed to enumerate modules." << std::endl;
        }
        CloseHandle(hProcess);
    }
}
