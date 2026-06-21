///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RiFuncDefs.h
/// Description  :  Function definitions for raw input.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>

typedef UINT    (*WNDPROC_CALLBACK)  (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef void    (*WNDPROC_ADDREM)    (WNDPROC_CALLBACK aWndProcCallback);
typedef LRESULT (*WNDPROC_SENDTOGAME)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
