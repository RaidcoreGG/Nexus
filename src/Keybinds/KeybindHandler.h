#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <Windows.h>
#include <mutex>
#include <map>
#include <string>

#include "FuncDefs.h"

#include "ActiveKeybind.h"
#include "Keybind.h"

namespace Keybinds
{
	extern std::mutex								Mutex;
	extern std::map<std::string, ActiveKeybind>		Registry;

	/* Keybind setting helpers */
	extern bool										IsSettingKeybind;
	extern Keybind									CurrentKeybind;
	extern std::string								CurrentKeybindUsedBy;

	extern std::map<unsigned short, std::string>	ScancodeLookupTable;

	/* Sets up the ScancodeLookupTable */
	void Initialize();

	/* Returns 0 if message was processed. */
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/* Loads the keybinds. */
	void Load();
	/* Saves the keybinds. */
	void Save();

	/* Generates and registers a keybind from the given string with the given identifier and handler, if no bind was previously stored the given one will be used. */
	void Register(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, const char* aKeybind);
	/* Registers a keybind from the given struct with the given identifier and handler, if no bind was previously stored the given one will be used.*/
	void RegisterWithStruct(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, Keybind aKeybind);
	/* Frees up the associated KeybindHandler. */
	void Unregister(const char* aIdentifier);
	
	/* Returns an empty string if keybind is unused or the identifier that uses this keybind */
	std::string IsInUse(Keybind aKeybind);
	/* Create a keybind from a string. */
	Keybind KBFromString(std::string aKeybind);
	
	/* This will force set the keybind. (Invoked via menu/ui) */
	void Set(std::string aIdentifier, Keybind aKeybind);
	/* Invokes the action on the corresponding keybind handler. Returns true if the keybind was dispatched. */
	bool Invoke(std::string aIdentifier);

	/* Removes all KeybindHandlers that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
};

#endif