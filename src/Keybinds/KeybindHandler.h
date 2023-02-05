#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <string>

#include "Keybind.h"

typedef void (*KEYBINDS_PROCESS)(std::string aIdentifier);
typedef void (*KEYBINDS_REGISTER)(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind);
typedef void (*KEYBINDS_UNREGISTER)(std::string aIdentifier);

namespace KeybindHandler
{
	extern std::mutex KeybindRegistryMutex;
	extern std::map<std::string, Keybind> KeybindRegistry;														/* Contains the active keybinds with their identifiers */
	extern std::map<std::string, KEYBINDS_PROCESS> KeybindHandlerRegistry;										/* Contains the owners of the keybind identifiers and their handler functions */

	extern std::mutex HeldKeysMutex;
	extern std::vector<WPARAM> HeldKeys;

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);											/* Returns true if handled */

	void LoadKeybinds();																						/* Loads the keybinds from disk */
	void SaveKeybinds();																						/* Saves the keybinds */

	void RegisterKeybind(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind);		/* Registers a kb with the given identifier, it will be passed to the given handler, if no bind was previously stored the given one will be used */
	void UnregisterKeybind(std::string aIdentifier);															/* This will free up the registered keybind event handler, should be called on addon shutdown */
	void SetKeybind(std::string aIdentifier, std::string aKeybind);												/* This will force set the keybind, invoked via menu/ui */
	void InvokeKeybind(std::string aIdentifier);																/* Invokes the action on the corresponding keybind handler */
};

#endif