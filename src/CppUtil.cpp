#include "WinSpy.h"
#include <windows.h>
// Required for process parameter structures
#include <winternl.h>
#include <string>
#include <vector>
#include <dbghelp.h>
#include <Psapi.h>
#include <wil/resource.h>

#pragma comment(lib, "dbghelp.lib")

// Function pointer types for dynamic loading
typedef NTSTATUS(NTAPI *pfnNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
);

BOOL GetProcessCommandLineWorker(DWORD dwProcessId, std::wstring& result)
{
    BOOL bSuccess = FALSE;
    HANDLE hProcess = NULL;
    HMODULE hNtDll = NULL;
    pfnNtQueryInformationProcess fnQueryProcess = NULL;

    result.clear();

    // Load ntdll.dll
    hNtDll = LoadLibrary(L"ntdll.dll");
    if (!hNtDll)
        return FALSE;
        
    // Get the function pointer
    fnQueryProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");
    if (!fnQueryProcess) {
        FreeLibrary(hNtDll);
        return FALSE;
    }
    
    // Open the process with query information rights
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
    if (!hProcess) {
        FreeLibrary(hNtDll);
        return FALSE;
    }
    
    // Get the process command line
    PVOID rtlUserProcParamsAddress = NULL;
    UNICODE_STRING commandLine;
    
    // Get the PEB address
    PROCESS_BASIC_INFORMATION pbi;
    NTSTATUS status = fnQueryProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);
    
    if (NT_SUCCESS(status)) {
        // Read the address of process parameters from PEB
        if (ReadProcessMemory(hProcess, (PVOID)((LPBYTE)pbi.PebBaseAddress + offsetof(PEB, ProcessParameters)), &rtlUserProcParamsAddress, sizeof(PVOID), NULL)) {
            // Read the command line from RTL_USER_PROCESS_PARAMETERS
            if (ReadProcessMemory(hProcess, (PVOID)((LPBYTE)rtlUserProcParamsAddress + offsetof(RTL_USER_PROCESS_PARAMETERS, CommandLine)), &commandLine, sizeof(commandLine), NULL)) {
                // Finally, read the actual command line
                if (commandLine.Length > 0 && commandLine.Buffer != NULL) {
                    const size_t lengthInChars = commandLine.Length / sizeof(WCHAR);
                    if (lengthInChars > 0) {
                        result.reserve(lengthInChars);
                        result.resize(result.capacity());
                        if (ReadProcessMemory(hProcess, commandLine.Buffer, &result[0], commandLine.Length, NULL)) {
                            bSuccess = TRUE;
                        }
                    }
                }
            }
        }
    }
    
    if (hProcess)
        CloseHandle(hProcess);
    
    if (hNtDll)
        FreeLibrary(hNtDll);
    
    return bSuccess;
}

struct ProcessCachedInfo
{
    DWORD dwProcessId;
    std::wstring commandLine;
};

std::vector<ProcessCachedInfo> g_processCache;

//
// Retrieves the command line used to start a process.
// Returns a pointer to the command line string.  Just use it immediately and don't free it, throw away the pointer.
//
extern "C" WCHAR* GetProcessCommandLine(DWORD dwProcessId)
{
    std::wstring* cmdLine {nullptr};

    // Check if g_processCache already has the command line for this process
    for (auto& processInfo : g_processCache)
    {
        if (processInfo.dwProcessId == dwProcessId)
        {
            cmdLine = &(processInfo.commandLine);
            break;
        }
    }

    if (!cmdLine)
    {
        auto& processCacheInfo = g_processCache.emplace_back(ProcessCachedInfo{ dwProcessId, L"" });

        // If not in cache, retrieve the command line
        if (!GetProcessCommandLineWorker(dwProcessId, processCacheInfo.commandLine))
        {
            g_processCache.pop_back(); // Remove the entry if we failed to get the command line
            return L"Error retrieving command line";
        }

        cmdLine = &(processCacheInfo.commandLine);
    }

    // How is this safe?  Well, it's only really safe until the std::vector is resized.
    // The caller needs to just use the pointer immediately and not keep it around.
    return cmdLine->data();
}

struct ProcInfo
{
    std::wstring ImagePath;
    std::string ModuleNameFromExportDir;
    std::wstring ErrorMessage;
};

std::vector<ProcInfo> g_procInfoCacheItems;

extern "C" CHAR* GetModuleNameFromExportDir(DWORD processId)
{
    // Get image path of procInfo.ProcessId
    wil::unique_handle process(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId));
    if (process == NULL) {
        return nullptr;
    }

    // Get the image file name
    WCHAR peFilePath[MAX_PATH];
    if (GetModuleFileNameExW(process.get(), NULL, peFilePath, MAX_PATH) == 0) {
        return nullptr;
    }

    for (auto &procInfo : g_procInfoCacheItems)
    {
        if (procInfo.ImagePath == peFilePath)
        {
            return procInfo.ModuleNameFromExportDir.data();
        }
    }

    auto &procInfo = g_procInfoCacheItems.emplace_back(std::move(ProcInfo{ peFilePath, "", L"" }));

    // Map file into memory
    wil::unique_hfile file(CreateFile(peFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    if (file.get() == INVALID_HANDLE_VALUE) {
        procInfo.ErrorMessage = L"Failed to open file";
        return nullptr;
    }
    wil::unique_handle mapping(CreateFileMapping(file.get(), NULL, PAGE_READONLY, 0, 0, NULL));
    if (mapping == nullptr) {
        procInfo.ErrorMessage = L"Failed to create file mapping";
        return nullptr;
    }

    wil::unique_any_handle_null<decltype(&::UnmapViewOfFile), ::UnmapViewOfFile> fileView(MapViewOfFile(mapping.get(), FILE_MAP_READ, 0, 0, 0));
    if (!fileView) {
        procInfo.ErrorMessage = L"Failed to map view of file";
        return nullptr;
    }

    ULONG size{ 0 };
    PIMAGE_SECTION_HEADER header;

    void *dirEntry = ::ImageDirectoryEntryToDataEx(
        fileView.get(),
        FALSE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &size,
        &header);

    // Get the exports
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)dirEntry;
    if (pExportDir == NULL) {
        procInfo.ErrorMessage = L"Failed to get export directory";
        return nullptr;
    }

    // Get PE headers for RVA to VA conversion
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)fileView.get();
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE *)fileView.get() + dosHeader->e_lfanew);

    // Get module name from export directory
    char *moduleName = (char *)ImageRvaToVa(
        ntHeaders,
        fileView.get(),
        pExportDir->Name,
        NULL);
    
    procInfo.ModuleNameFromExportDir = std::string(moduleName);

    return procInfo.ModuleNameFromExportDir.data();
}
