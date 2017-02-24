//
//	GetRemoteClassInfoEx.c
//
//  Copyright (c) 2002 by J Brown 
//  Freeware
//
//  BOOL GetRemoteClassInfoEx(HWND hwnd)
//
//  In order to retrieve private class information for a 
//  window in another process, we have to create
//  a remote thread in that process and call GetClassInfoEx from
//  there.
//
//  GetRemoteClassInfoEx uses the InjectRemoteThread call defined
//  in InjectThread.c
//

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#include "InjectThread.h"

typedef BOOL(WINAPI *PROCGETCLASSINFOEX)(HINSTANCE, LPCTSTR, WNDCLASSEX*);
typedef LONG_PTR(WINAPI *PROCGETWINDOWLONGPTR)(HWND, int);
typedef LRESULT(WINAPI *PROCSENDMESSAGETO)(HWND, UINT, WPARAM, LPARAM, UINT, UINT, PDWORD_PTR);

//
//	Define a structure for the remote thread to use
//
typedef struct
{
	PROCGETCLASSINFOEX    fnGetClassInfoEx;
	PROCGETWINDOWLONGPTR  fnGetWindowLongPtr;
	PROCSENDMESSAGETO     fnSendMessageTimeout;

	HWND        hwnd; //window we want to get class info for
	ATOM        atom; //class atom of window
	HINSTANCE   hInst;

	WNDCLASSEX  wcOutput;
	WNDPROC     wndproc;

	// Window text to retrieve
	TCHAR       szText[200]; // text (out)
	int         nTextSize;

} INJDATA;

#pragma runtime_checks("", off)
// calls to the stack checking routine must be disabled
#pragma check_stack(off)

// From https://msdn.microsoft.com/en-us/library/7977wcck.aspx:
// The order here is important.
// Section names must be 8 characters or less.
// The sections with the same name before the $
// are merged into one section. The order that
// they are merged is determined by sorting
// the characters after the $.
// InitSegStart and InitSegEnd are used to set
// boundaries so we can find the real functions
// that we need to call for initialization.

//
//	Thread to inject to remote process. Must not
//  make ANY calls to code in THIS process.
//
__declspec(code_seg(".inject$a"))
static DWORD WINAPI GetClassInfoExProc(LPVOID *pParam)
{
	INJDATA *pInjData = (INJDATA *)pParam;
	BOOL    fRet = 0;
	DWORD_PTR dwpResult;

	if (pInjData->fnGetWindowLongPtr)
		pInjData->wndproc = (WNDPROC)pInjData->fnGetWindowLongPtr(pInjData->hwnd, GWLP_WNDPROC);

	if (pInjData->fnGetClassInfoEx)
		fRet = pInjData->fnGetClassInfoEx(pInjData->hInst, (LPCTSTR)pInjData->atom, &pInjData->wcOutput);

	if (pInjData->fnSendMessageTimeout)
	{
		// Null-terminate in case the gettext fails
		pInjData->szText[0] = _T('\0');

		pInjData->fnSendMessageTimeout(pInjData->hwnd, WM_GETTEXT,
			pInjData->nTextSize, (LPARAM)pInjData->szText,
			SMTO_ABORTIFHUNG, 100, &dwpResult);
	}

	return fRet;

}

__declspec(code_seg(".inject$z"))
static void AfterThreadProc(void) { }

#pragma check_stack
#pragma runtime_checks("", restore)

BOOL GetRemoteWindowInfo(HWND hwnd, WNDCLASSEX *pClass, WNDPROC *pProc, TCHAR *pszText, int nTextLen)
{
	INJDATA InjData;
	BOOL    fReturn;

	// Calculate how many bytes the injected code takes
	DWORD_PTR cbCodeSize = ((BYTE *)AfterThreadProc - (BYTE *)GetClassInfoExProc);

	//
	// Setup the injection structure:
	//
	ZeroMemory(&InjData, sizeof(InjData));

	// Get pointers to the API calls we will be using in the remote thread
	InjData.fnSendMessageTimeout = SendMessageTimeout;

	InjData.fnGetWindowLongPtr = IsWindowUnicode(hwnd) ? GetWindowLongPtrW : GetWindowLongPtrA;
	InjData.fnGetClassInfoEx = IsWindowUnicode(hwnd) ? GetClassInfoExW : (PROCGETCLASSINFOEX)GetClassInfoExA;

	// Setup the data the API calls will need
	InjData.hwnd = (HWND)hwnd;
	InjData.atom = (ATOM)GetClassLong(hwnd, GCW_ATOM);
	InjData.hInst = (HINSTANCE)GetClassLongPtr(hwnd, GCLP_HMODULE);
	InjData.wndproc = 0;
	InjData.nTextSize = ARRAYSIZE(InjData.szText);

	//
	// Inject the GetClassInfoExProc function, and our InjData structure!
	//
	fReturn = InjectRemoteThread(hwnd, GetClassInfoExProc, cbCodeSize, &InjData, sizeof(InjData));

	if (fReturn == FALSE)
	{
		// Failed to retrieve class information!
		*pProc = NULL;
		ZeroMemory(pClass, sizeof(WNDCLASSEX));
		pszText[0] = 0;
		return FALSE;
	}
	else
	{
		*pClass = InjData.wcOutput;
		// As these pointers come from another process, zero them out to avoid accidental misuse
		pClass->lpszClassName = pClass->lpszMenuName = NULL;
		*pProc = InjData.wndproc;

		lstrcpyn(pszText, InjData.szText, 200);
		return TRUE;
	}
}
