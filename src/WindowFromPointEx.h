#ifndef WINDOWFROMPOINT_INCLUDED
#define WINDOWFROMPOINT_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

HWND WindowFromPointEx(POINT pt, BOOL fTopLevel, BOOL fAllowHidden);

#ifdef __cplusplus
}
#endif

#endif
