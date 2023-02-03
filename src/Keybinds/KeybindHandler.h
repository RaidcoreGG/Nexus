#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <string>

#include "Keybind.h"

typedef void (*KEYBINDS_PROCESS)(std::wstring aIdentifier);
typedef void (*KEYBINDS_REGISTER)(std::wstring aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::wstring aKeybind);
typedef void (*KEYBINDS_UNREGISTER)(std::wstring aIdentifier);

namespace KeybindHandler
{
	extern std::mutex KeybindRegistryMutex;
	extern std::map<std::wstring, Keybind> KeybindRegistry;														/* Contains the active keybinds with their identifiers */
	extern std::map<std::wstring, KEYBINDS_PROCESS> KeybindHandlerRegistry;										/* Contains the owners of the keybind identifiers and their handler functions */

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);											/* Returns true if handled */

	void LoadKeybinds();																						/* Loads the keybinds from disk */
	void SaveKeybinds();																						/* Saves the keybinds */

	void RegisterKeybind(std::wstring aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::wstring aKeybind);	/* Registers a kb with the given identifier, it will be passed to the given handler, if no bind was previously stored the given one will be used */
	void UnregisterKeybind(std::wstring aIdentifier);															/* This will free up the registered keybind event handler, should be called on addon shutdown */
	void SetKeybind(std::wstring aIdentifier, std::wstring aKeybind);											/* This will force set the keybind, invoked via menu/ui */
	void InvokeKeybind(std::wstring aIdentifier);																/* Invokes the action on the corresponding keybind handler */
};

#endif