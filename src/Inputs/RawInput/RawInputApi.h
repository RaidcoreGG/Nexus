///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RawInputApi.h
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef RAWINPUTAPI_H
#define RAWINPUTAPI_H

#include <vector>
#include <mutex>

#include "FuncDefs.h"

#define WM_PASSTHROUGH_FIRST WM_USER + 7997
#define WM_PASSTHROUGH_LAST  WM_USER + 7997 + WM_USER - 1

///----------------------------------------------------------------------------------------------------
/// RawInput Namespace
///----------------------------------------------------------------------------------------------------
namespace RawInput
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_SendWndProcToGame:
	/// 	Skips all WndProc callbacks and sends it directly to the original.
	///----------------------------------------------------------------------------------------------------
	LRESULT ADDONAPI_SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Register:
	/// 	Registers the provided WndProcCallback.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_Register(WNDPROC_CALLBACK aWndProcCallback);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Deregister:
	/// 	Deregisters the provided WndProcCallback.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_Deregister(WNDPROC_CALLBACK aWndProcCallback);
}

///----------------------------------------------------------------------------------------------------
/// CRawInputApi Class
///----------------------------------------------------------------------------------------------------
class CRawInputApi
{
public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CRawInputApi() = default;
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CRawInputApi() = default;

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed or non-zero, if it should be passed to the next callback.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// SendWndProcToGame:
	/// 	Skips all WndProc callbacks and sends it directly to the original.
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

private:
	mutable std::mutex				Mutex;
	std::vector<WNDPROC_CALLBACK>	Registry;
};

#endif
