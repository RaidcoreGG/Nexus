#ifndef KEYBINDHANDLER_H
#define KEYBINDHANDLER_H

#include <Windows.h>
#include <map>
#include <vector>
#include <mutex>

typedef void (*KeybindHandlerSig)(const wchar_t*);

struct Keybind
{
	WORD Key;
	bool Alt;
	bool Ctrl;
	bool Shift;
};

namespace KeybindHandler
{
	bool WndProc(HWND, UINT, WPARAM, LPARAM);									/* Returns true if handled */

	void LoadKeybinds();
	void SaveKeybinds();

	void RegisterKeybind(const wchar_t*, KeybindHandlerSig, Keybind);			/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void InvokeKeybind(const wchar_t*);											/* Passes the action to the corresponding addon to handle it */
	
	static std::mutex KeybindRegistryMutex;
	static std::map<const wchar_t*, Keybind> KeybindRegistry;					/* Contains the active keybinds with their identifiers */
	static std::map<const wchar_t*, KeybindHandlerSig> KeybindHandlerRegistry;	/* Contains the owners of the keybind identifiers and their handler functions */
};

#endif