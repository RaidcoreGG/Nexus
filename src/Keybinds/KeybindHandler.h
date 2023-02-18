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
#include "FuncDefs.h"

namespace Keybinds
{
	extern std::mutex Mutex;
	extern std::map<std::string, Keybind> Registry;														/* Contains the active keybinds with their identifiers */
	extern std::map<std::string, KEYBINDS_PROCESS> HandlerRegistry;										/* Contains the owners of the keybind identifiers and their handler functions */

	extern std::mutex HeldKeysMutex;
	extern std::vector<WPARAM> HeldKeys;

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);									/* Returns true if handled */

	void Load();																						/* Loads the keybinds from disk */
	void Save();																						/* Saves the keybinds */

	void Register(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind);		/* Registers a kb with the given identifier, it will be passed to the given handler, if no bind was previously stored the given one will be used */
	void Unregister(std::string aIdentifier);															/* This will free up the registered keybind event handler, should be called on addon shutdown */
	void Set(std::string aIdentifier, std::string aKeybind);											/* This will force set the keybind, invoked via menu/ui */
	void Invoke(std::string aIdentifier);																/* Invokes the action on the corresponding keybind handler */
};

#endif