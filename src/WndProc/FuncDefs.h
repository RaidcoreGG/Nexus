#ifndef WNDPROC_FUNCDEFS_H
#define WNDPROC_FUNCDEFS_H

#include <Windows.h>

typedef UINT (*WNDPROC_CALLBACK)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef void (*WNDPROC_ADDREM)(WNDPROC_CALLBACK aWndProcCallback);

#endif