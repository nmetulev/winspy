#include "WinSpy.h"
#include <windows.h>
// Required for process parameter structures
#include <winternl.h>
#include <string>
#include <vector>

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
