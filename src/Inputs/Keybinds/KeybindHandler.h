///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  KeybindHandler.h
/// Description  :  Provides functions for keybinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <Windows.h>
#include <mutex>
#include <map>
#include <string>
#include <vector>

#include "FuncDefs.h"

#include "ActiveKeybind.h"
#include "Keybind.h"

///----------------------------------------------------------------------------------------------------
/// Keybinds Namespace
///----------------------------------------------------------------------------------------------------
namespace Keybinds
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RegisterWithString:
	/// 	[Revision 1] Addon API wrapper function for Register from string.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RegisterWithString(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, const char* aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RegisterWithStruct:
	/// 	[Revision 1] Addon API wrapper function for Register from struct.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RegisterWithStruct(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, Keybind aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RegisterWithString:
	/// 	[Revision 2] Addon API wrapper function for Register from string.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RegisterWithString2(const char* aIdentifier, KEYBINDS_PROCESS2 aKeybindHandler, const char* aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RegisterWithStruct:
	/// 	[Revision 2] Addon API wrapper function for Register from struct.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RegisterWithStruct2(const char* aIdentifier, KEYBINDS_PROCESS2 aKeybindHandler, Keybind aKeybind);
}

///----------------------------------------------------------------------------------------------------
/// Keybinds Namespace
///----------------------------------------------------------------------------------------------------
namespace Keybinds
{
	extern std::mutex								Mutex;
	extern std::map<std::string, ActiveKeybind>		Registry;

	extern bool										IsSettingKeybind;
	extern Keybind									CurrentKeybind;
	extern std::string								CurrentKeybindUsedBy;

	/* this is ugly as fuck */
	extern bool										AltTracked;
	extern bool										CtrlTracked;
	extern bool										ShiftTracked;
	extern std::vector<unsigned short>				KeysTracked;
	extern std::map<std::string, ActiveKeybind>		HeldKeybinds;

	extern std::map<unsigned short, std::string>	ScancodeLookupTable;

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Sets up the ScancodeLookupTable and loads the keybinds.
	///----------------------------------------------------------------------------------------------------
	void Initialize();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if a keybind was invoked.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the keybinds.
	///----------------------------------------------------------------------------------------------------
	void Load();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the keybinds.
	///----------------------------------------------------------------------------------------------------
	void Save();

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Generates and registers a keybind from the given string with the given identifier and handler,
	/// 	if no bind was previously stored the given one will be used.
	///----------------------------------------------------------------------------------------------------
	void Register(const char* aIdentifier, EKeybindHandlerType aKeybindHandlerType, void* aKeybindHandler, const char* aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers a keybind from the given struct with the given identifier and handler,
	/// 	if no bind was previously stored the given one will be used.
	///----------------------------------------------------------------------------------------------------
	void Register(const char* aIdentifier, EKeybindHandlerType aKeybindHandlerType, void* aKeybindHandler, Keybind aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters a KeybindHandler from an identifier.
	///----------------------------------------------------------------------------------------------------
	void Deregister(const char* aIdentifier);
	
	///----------------------------------------------------------------------------------------------------
	/// IsInUse:
	/// 	Returns an empty string if keybind is unused or the identifier that uses this keybind.
	///----------------------------------------------------------------------------------------------------
	std::string IsInUse(Keybind aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// KBFromString:
	/// 	Helper function to create a keybind from a string.
	///----------------------------------------------------------------------------------------------------
	Keybind KBFromString(std::string aKeybind);
	
	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Sets a keybind.
	///----------------------------------------------------------------------------------------------------
	void Set(std::string aIdentifier, Keybind aKeybind);

	///----------------------------------------------------------------------------------------------------
	/// Invoke:
	/// 	Invokes the action on the corresponding keybind handler.
	/// 	Returns true if the keybind was dispatched.
	///----------------------------------------------------------------------------------------------------
	bool Invoke(std::string aIdentifier, bool aIsRelease = false);

	///----------------------------------------------------------------------------------------------------
	/// Deletes:
	/// 	Deletes a keybind entirely.
	///----------------------------------------------------------------------------------------------------
	void Delete(std::string aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all KeybindHandlers that are within the provided address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);
};

#endif
