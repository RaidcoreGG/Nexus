///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WndProcHandler.h
/// Description  :  Proxy for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef WNDPROCHANDLER_H
#define WNDPROCHANDLER_H

#include <vector>
#include <mutex>

#include "FuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// WndProc Namespace
///----------------------------------------------------------------------------------------------------
namespace WndProc
{
	extern std::mutex						Mutex;
	extern std::vector<WNDPROC_CALLBACK>	Registry;

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed or non-zero, if it should be passed to the next callback.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// SendWndProcToGame:
	/// 	Skips all WndProc callbacks and sends to game only.
	///----------------------------------------------------------------------------------------------------
	LRESULT SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers the provided WndProcCallback.
	///----------------------------------------------------------------------------------------------------
	void Register(WNDPROC_CALLBACK aWndProcCallback);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters the provided WndProcCallback.
	///----------------------------------------------------------------------------------------------------
	void Deregister(WNDPROC_CALLBACK aWndProcCallback);

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all WndProc Callbacks that are within the provided address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif
