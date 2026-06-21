///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EscapeClosing.h
/// Description  :  Contains the functionality to close windows on escape.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <windows.h>

#include "Engine/_Concepts/IWndProc.h"
#include "Engine/Cleanup/RefCleanerBase.h"

///----------------------------------------------------------------------------------------------------
/// CEscapeClosing Class
///----------------------------------------------------------------------------------------------------
class CEscapeClosing : public virtual IRefCleaner, public virtual IWndProc
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CEscapeClosing();

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CEscapeClosing();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers the matching window name to be toggled on escape.
	///----------------------------------------------------------------------------------------------------
	void Register(const char* aWindowName, bool* aIsVisible);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters the matching window name from being toggled on escape.
	///----------------------------------------------------------------------------------------------------
	void Deregister(const char* aWindowName);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters the matching bool pointer from being toggled on escape.
	///----------------------------------------------------------------------------------------------------
	void Deregister(bool* aIsVisible);

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes all registered close-on-escape hooks that match the address space.
	///----------------------------------------------------------------------------------------------------
	uint32_t CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	private:
	std::mutex                             Mutex;
	std::unordered_map<std::string, bool*> Registry;

	bool                                   Enabled = true;
};
