///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RiApi.h
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <vector>

#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/Renderer/RdrContext.h"
#include "RiFuncDefs.h"

#define WM_PASSTHROUGH_FIRST WM_USER + 7997
#define WM_PASSTHROUGH_LAST  WM_USER + 7997 + WM_USER - 1

///----------------------------------------------------------------------------------------------------
/// CRawInputApi Class
///----------------------------------------------------------------------------------------------------
class CRawInputApi : public virtual IRefCleaner
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CRawInputApi(RenderContext_t* aRenderCtx);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CRawInputApi();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed or non-zero, if it should be passed to the next callback.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// WndProcGameOnly:
	/// 	Returns the uMsg shifted back to the normal range.
	/// 	This should be called after all other window procedures.
	///----------------------------------------------------------------------------------------------------
	UINT WndProcGameOnly(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
	/// CleanupRefs:
	/// 	Removes all WndProc Callbacks that are within the provided address space.
	///----------------------------------------------------------------------------------------------------
	uint32_t CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	private:
	RenderContext_t*              RenderContext = nullptr;

	mutable std::mutex            Mutex;
	std::vector<WNDPROC_CALLBACK> Registry;
};
