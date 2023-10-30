#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdarg>

#include "../core.h"
#include "../Paths.h"
#include "../Shared.h"
#include "../State.h"

#include "Keybind.h"
#include "ActiveKeybind.h"
#include "FuncDefs.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

namespace Keybinds
{
	extern std::mutex								Mutex;
	extern std::map<std::string, ActiveKeybind>		Registry;

	/* Keybind Helpers */
	extern std::mutex								HeldKeysMutex;
	extern std::vector<WPARAM>						HeldKeys;

	/* Keybind setting helpers */
	extern bool										IsSettingKeybind;
	extern Keybind									CurrentKeybind;
	extern std::string								CurrentKeybindUsedBy;

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
	
	/* This will force set the keybind. (Invoked via menu/ui) */
	void Set(std::string aIdentifier, Keybind aKeybind);
	/* Invokes the action on the corresponding keybind handler. Returns true if the keybind was dispatched. */
	bool Invoke(std::string aIdentifier);

	/* Removes all KeybindHandlers that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
};

#endif