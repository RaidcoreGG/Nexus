///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IWndProc.cpp
/// Description  :  Interface for a uniform WndProc callback.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>

///----------------------------------------------------------------------------------------------------
/// IWndProc Interface Class
///----------------------------------------------------------------------------------------------------
class IWndProc
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed and should not be processed by any other handler.
	///----------------------------------------------------------------------------------------------------
	virtual UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};
