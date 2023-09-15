#ifndef KEYBINDS_FUNCDEFS_H
#define KEYBINDS_FUNCDEFS_H

#include <string>

typedef void (*KEYBINDS_PROCESS)(const char* aIdentifier);
typedef void (*KEYBINDS_REGISTER)(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, const char* aKeybind);
typedef void (*KEYBINDS_UNREGISTER)(const char* aIdentifier);
typedef bool (*ADDON_WNDPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef void (*KEYBINDS_REGISTERWNDPROC)(ADDON_WNDPROC aWndProcCallback);
typedef void (*KEYBINDS_UNREGISTERWNDPROC)(ADDON_WNDPROC aWndProcCallback);

#endif