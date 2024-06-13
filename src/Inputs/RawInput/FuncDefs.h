#ifndef RAWINPUT_FUNCDEFS_H
#define RAWINPUT_FUNCDEFS_H

#include <Windows.h>

typedef UINT	(*WNDPROC_CALLBACK)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef void	(*WNDPROC_ADDREM)(WNDPROC_CALLBACK aWndProcCallback);
typedef LRESULT	(*WNDPROC_SENDTOGAME)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif