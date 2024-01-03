#ifndef ADDONAPI_H
#define ADDONAPI_H

#include <dxgi.h>
#include "Paths/FuncDefs.h"
#include "Logging/FuncDefs.h"
#include "Events/FuncDefs.h"
#include "WndProc/FuncDefs.h"
#include "Keybinds/FuncDefs.h"
#include "imgui/imgui.h"
#include "minhook/mh_hook.h"
#include "DataLink/FuncDefs.h"
#include "Textures/FuncDefs.h"
#include "GUI/FuncDefs.h"
#include "GUI/Widgets/QuickAccess/FuncDefs.h"

// Base
struct AddonAPI {};

// Revision 1
struct AddonAPI1 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*				SwapChain;
	ImGuiContext*				ImguiContext;
	void*						ImguiMalloc;
	void*						ImguiFree;
	GUI_ADDRENDER				RegisterRender;
	GUI_REMRENDER				DeregisterRender;

	/* Paths */
	PATHS_GETGAMEDIR			GetGameDirectory;
	PATHS_GETADDONDIR			GetAddonDirectory;
	PATHS_GETCOMMONDIR			GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE				CreateHook;
	MINHOOK_REMOVE				RemoveHook;
	MINHOOK_ENABLE				EnableHook;
	MINHOOK_DISABLE				DisableHook;

	/* Logging */
	LOGGER_LOGA					Log;

	/* Events */
	EVENTS_RAISE				RaiseEvent;
	EVENTS_SUBSCRIBE			SubscribeEvent;
	EVENTS_SUBSCRIBE			UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM				RegisterWndProc;
	WNDPROC_ADDREM				DeregisterWndProc;

	/* Keybinds */
	KEYBINDS_REGISTERWITHSTRING	RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT	RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER			DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE		GetResource;
	DATALINK_SHARERESOURCE		ShareResource;

	/* Textures */
	TEXTURES_GET				GetTexture;
	TEXTURES_LOADFROMFILE		LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE	LoadTextureFromResource;
	TEXTURES_LOADFROMURL		LoadTextureFromURL;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT		AddShortcut;
	QUICKACCESS_GENERIC			RemoveShortcut;
	QUICKACCESS_ADDSIMPLE		AddSimpleShortcut;
	QUICKACCESS_GENERIC			RemoveSimpleShortcut;
};

struct AddonAPI2 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*				SwapChain;
	ImGuiContext*				ImguiContext;
	void*						ImguiMalloc;
	void*						ImguiFree;
	GUI_ADDRENDER				RegisterRender;
	GUI_REMRENDER				DeregisterRender;

	/* Paths */
	PATHS_GETGAMEDIR			GetGameDirectory;
	PATHS_GETADDONDIR			GetAddonDirectory;
	PATHS_GETCOMMONDIR			GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE				CreateHook;
	MINHOOK_REMOVE				RemoveHook;
	MINHOOK_ENABLE				EnableHook;
	MINHOOK_DISABLE				DisableHook;

	/* Logging */
	LOGGER_LOGA					Log;

	/* Events */
	EVENTS_RAISE				RaiseEvent;
	EVENTS_SUBSCRIBE			SubscribeEvent;
	EVENTS_SUBSCRIBE			UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM				RegisterWndProc;
	WNDPROC_ADDREM				DeregisterWndProc;

	/* Keybinds */
	KEYBINDS_REGISTERWITHSTRING	RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT	RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER			DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE		GetResource;
	DATALINK_SHARERESOURCE		ShareResource;

	/* Textures */
	TEXTURES_GET				GetTexture;
	TEXTURES_LOADFROMFILE		LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE	LoadTextureFromResource;
	TEXTURES_LOADFROMURL		LoadTextureFromURL;
	TEXTURES_LOADFROMMEMORY		LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT		AddShortcut;
	QUICKACCESS_GENERIC			RemoveShortcut;
	QUICKACCESS_GENERIC			NotifyShortcut;
	QUICKACCESS_ADDSIMPLE		AddSimpleShortcut;
	QUICKACCESS_GENERIC			RemoveSimpleShortcut;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
};

#endif