#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <Windows.h>
#include <map>
#include <vector>
#include <mutex>
#include <iostream>
#include <fstream>

#include "../nlohmann/json.hpp"

struct Keybind
{
	WORD Key;
	bool Alt;
	bool Ctrl;
	bool Shift;
};

bool operator==(const Keybind&, const Keybind&);

typedef void (*KeybindHandlerSig)(const wchar_t*);
typedef void (*RegisterKeybindSig)(const wchar_t*, KeybindHandlerSig, Keybind);

namespace KeybindHandler
{
	bool WndProc(HWND, UINT, WPARAM, LPARAM);									/* Returns true if handled */

	void LoadKeybinds();														/* Loads the keybinds from disk */
	void SaveKeybinds();														/* Saves the keybinds */
	void ValidateKeybinds();													/* Checks for stale keybinds */

	void RegisterKeybind(const wchar_t*, KeybindHandlerSig, Keybind);			/* Registers a kb with the given identifier, it will be passed to the given handler, if no bind was previously stored the given one will be used */
	void InvokeKeybind(const wchar_t*);											/* Invokes the action on the corresponding keybind handler */
	
	static std::mutex KeybindRegistryMutex;
	static std::map<const wchar_t*, Keybind> KeybindRegistry;					/* Contains the active keybinds with their identifiers */
	static std::map<const wchar_t*, KeybindHandlerSig> KeybindHandlerRegistry;	/* Contains the owners of the keybind identifiers and their handler functions */

	static std::wfstream File;
};

#endif