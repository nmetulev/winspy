//
//  InjectThread.c
//
//  Copyright (c) 2002 by J Brown
//  Freeware
//
//  InjectThread uses the CreateRemoteThread API call
//  (under NT/2000/XP) to inject a piece of binary code
//  into the process which owns the specified window.
//
//

#include "WinSpy.h"

#include "InjectThread.h"

#define INJECT_ACCESS (PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE)

typedef PVOID(WINAPI * VA_EX_PROC)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
typedef PVOID(WINAPI * VF_EX_PROC)(HANDLE, LPVOID, SIZE_T, DWORD);

//
//  Inject a thread into the process which owns the specified window.
//
//  lpCode     - address of function to inject. (See CreateRemoteThread)
//  cbCodeSize - size (in bytes) of the function
//
//  lpData     - address of a user-defined structure to be passed to the injected thread
//  cbDataSize - size (in bytes) of the structure
//
//  The user-defined structure is also injected into the target process' address space.
//  When the thread terminates, the structure is read back from the process.
//
DWORD InjectRemoteThread(HWND hwnd, LPTHREAD_START_ROUTINE lpCode, DWORD_PTR cbCodeSize, LPVOID lpData, DWORD cbDataSize)
{
	DWORD  dwProcessId;         //id of remote process
	DWORD  dwThreadId;          //id of the thread in remote process
	HANDLE hProcess;            //handle to the remote process

	HANDLE hRemoteThread;       //handle to the injected thread

	SIZE_T dwWritten;           // Number of bytes written to the remote process
	SIZE_T dwRead;
	DWORD  dwExitCode;

	void *pRemoteCode;
	void *pRemoteData;

	const DWORD_PTR cbCodeSizeAligned = (cbCodeSize + (sizeof(LONG_PTR) - 1)) & ~(sizeof(LONG_PTR) - 1);

	// Return FALSE in case of failure
	dwExitCode = FALSE;

	// Find the process ID of the process which created the specified window
	dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);

	// Open the remote process so we can allocate some memory in it
	hProcess = OpenProcess(INJECT_ACCESS, FALSE, dwProcessId);
	if (hProcess)
	{
		pRemoteCode = VirtualAllocEx(hProcess, 0, cbCodeSizeAligned + cbDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (pRemoteCode)
		{
			// Write a copy of our injection thread into the remote process
			WriteProcessMemory(hProcess, pRemoteCode, lpCode, cbCodeSize, &dwWritten);

			// Write a copy of the INJTHREAD to the remote process. This structure
			// MUST start on a 32bit/64bit boundary
			pRemoteData = (void *)((BYTE *)pRemoteCode + cbCodeSizeAligned);

			// Put DATA in the remote thread's memory block
			WriteProcessMemory(hProcess, pRemoteData, lpData, cbDataSize, &dwWritten);

			// Create the remote thread!!!
			hRemoteThread = CreateRemoteThread(hProcess, NULL, 0,
				(LPTHREAD_START_ROUTINE)pRemoteCode, pRemoteData, 0, NULL);

			if (hRemoteThread)
			{
				// Wait for the thread to terminate
				if (WaitForSingleObject(hRemoteThread, 7000) != WAIT_OBJECT_0)
				{
					// Timeout or failure
					// Do not call VirtualFreeEx as the code may still run in the future
					CloseHandle(hRemoteThread);
					CloseHandle(hProcess);

					return FALSE;
				}

				// Read the user-structure back again
				ReadProcessMemory(hProcess, pRemoteData, lpData, cbDataSize, &dwRead);

				GetExitCodeThread(hRemoteThread, &dwExitCode);

				CloseHandle(hRemoteThread);
			}

			// Free the memory in the remote process
			VirtualFreeEx(hProcess, pRemoteCode, 0, MEM_RELEASE);
		}

		CloseHandle(hProcess);
	}

	return dwExitCode;
}
