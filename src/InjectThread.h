#ifndef INJECT_THREAD
#define INJECT_THREAD

#ifdef __cplusplus
extern "C" {
#endif

DWORD InjectRemoteThread(HWND hwnd, LPTHREAD_START_ROUTINE lpCode, DWORD_PTR cbCodeSize, LPVOID lpData, DWORD cbDataSize, DWORD cbInput);

#ifdef __cplusplus
}
#endif

#endif