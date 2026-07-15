///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Hooks.h
/// Description  :  Implementation for hooked/detoured functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <windows.h>

#include "HkFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// Hooks Namespace
///----------------------------------------------------------------------------------------------------
namespace Hooks
{
	///----------------------------------------------------------------------------------------------------
	/// HookIDXGISwapChain:
	/// 	Hooks IDXGISwapChain::Present and IDXGISwapChain::ResizeBuffers.
	/// 	Checks if:
	/// 		- the process is not run with "-ggvanilla".
	/// 		- they are not hooked already.
	///----------------------------------------------------------------------------------------------------
	void HookIDXGISwapChain();

	///----------------------------------------------------------------------------------------------------
	/// Hooks::Target Namespace
	/// 	Stores the original functions.
	///----------------------------------------------------------------------------------------------------
	namespace Target
	{
		extern WNDPROC          WndProc;
		extern DXPRESENT        DXGIPresent;
		extern DXPRESENT1       DXGIPresent1;
		extern DXRESIZEBUFFERS  DXGIResizeBuffers;
		extern DXRESIZEBUFFERS1 DXGIResizeBuffers1;
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
		/// (DETOUR) IDXGISwapChain::Present
		///----------------------------------------------------------------------------------------------------
		HRESULT __stdcall DXGIPresent(
			IDXGISwapChain* pChain,
			UINT            SyncInterval,
			UINT            Flags
		);

		///----------------------------------------------------------------------------------------------------
		/// (DETOUR) IDXGISwapChain1::Present1
		///----------------------------------------------------------------------------------------------------
		HRESULT __stdcall DXGIPresent1(
			IDXGISwapChain1*               pChain,
			UINT                           SyncInterval,
			UINT                           PresentFlags,
			const DXGI_PRESENT_PARAMETERS* pPresentParameters
		);

		///----------------------------------------------------------------------------------------------------
		/// (DETOUR) IDXGISwapChain::ResizeBuffers
		///----------------------------------------------------------------------------------------------------
		HRESULT __stdcall DXGIResizeBuffers(
			IDXGISwapChain* pChain,
			UINT            BufferCount,
			UINT            Width,
			UINT            Height,
			DXGI_FORMAT     NewFormat,
			UINT            SwapChainFlags
		);

		///----------------------------------------------------------------------------------------------------
		/// (DETOUR) IDXGISwapChain3::ResizeBuffers1
		///----------------------------------------------------------------------------------------------------
		HRESULT __stdcall DXGIResizeBuffers1(
			IDXGISwapChain3* pChain,
			UINT             BufferCount,
			UINT             Width,
			UINT             Height,
			DXGI_FORMAT      Format,
			UINT             SwapChainFlags,
			const UINT*      pCreationNodeMask,
			IUnknown* const* ppPresentQueue
		);
	}
}
