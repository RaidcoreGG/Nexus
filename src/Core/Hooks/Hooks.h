///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Hooks.h
/// Description  :  Implementation for hooked/detoured functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef HOOKS_H
#define HOOKS_H

#include <windows.h>

#include "HkFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// Hooks Namespace
///----------------------------------------------------------------------------------------------------
namespace Hooks
{
	///----------------------------------------------------------------------------------------------------
	/// Hooks::Target Namespace
	/// 	Stores the original functions.
	///----------------------------------------------------------------------------------------------------
	namespace Target
	{
		extern DXPRESENT       DXGIPresent;
		extern DXRESIZEBUFFERS DXGIResizeBuffers;
		extern WNDPROC         WndProc;
	}

	///----------------------------------------------------------------------------------------------------
	/// Hooks::Detour Namespace
	/// 	Definitions for detour functions.
	///----------------------------------------------------------------------------------------------------
	namespace Detour
	{
		///----------------------------------------------------------------------------------------------------
		/// (DETOUR) WndProc
		///----------------------------------------------------------------------------------------------------
		LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		///----------------------------------------------------------------------------------------------------
		/// (DETOUR) DXGIPresent
		///----------------------------------------------------------------------------------------------------
		HRESULT __stdcall DXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);

		///----------------------------------------------------------------------------------------------------
		/// (DETOUR) DXGIResizeBuffers
		///----------------------------------------------------------------------------------------------------
		HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	}
}

#endif
