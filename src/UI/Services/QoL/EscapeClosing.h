///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EscapeClosing.h
/// Description  :  Contains the functionality to close windows on escape.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ESCAPECLOSING_H
#define ESCAPECLOSING_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <Windows.h>

///----------------------------------------------------------------------------------------------------
/// CEscapeClosing Class
///----------------------------------------------------------------------------------------------------
class CEscapeClosing
{
	public:
	bool Enabled = true;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CEscapeClosing();

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CEscapeClosing() = default;

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
	/// Verify:
	/// 	Removes all registered close-on-escape hooks that match the address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	private:
	std::mutex								Mutex;
	std::unordered_map<std::string, bool*>	Registry;
};

#endif
