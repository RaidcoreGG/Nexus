///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RiApi.h
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "Engine/Inputs/RawInput/WndProcNotify.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "RiFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// CRawInputApi Class
///----------------------------------------------------------------------------------------------------
class CRawInputApi : public virtual IRefCleaner, public IWndProcNotifier
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CRawInputApi();

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CRawInputApi();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Passes the message to the addon callbacks, then to the native subscribers.
	/// 	Returns 0 if message was processed or non-zero, if it should be passed to the game.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
	/* Recursive and a snapshot is iterated, so a callback may (de)register itself. */
	std::recursive_mutex                                 Mutex;
	std::shared_ptr<const std::vector<WNDPROC_CALLBACK>> Registry = std::make_shared<const std::vector<WNDPROC_CALLBACK>>();
};
