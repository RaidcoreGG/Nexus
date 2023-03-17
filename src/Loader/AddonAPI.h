#ifndef ADDONAPI_H
#define ADDONAPI_H

#include <dxgi.h>
#include "../Mumble/LinkedMem.h"
#include "../Logging/FuncDefs.h"
#include "../Events/FuncDefs.h"
#include "../Keybinds/FuncDefs.h"
#include "../imgui/imgui.h"
#include "../minhook/mh_hook.h"
#include "../DataLink/FuncDefs.h"
#include "../Textures/FuncDefs.h"
#include "../GUI/Widgets/QuickAccess/FuncDefs.h"
#include "NexusLinkData.h"

using namespace Mumble;

struct AddonAPI
{
	/* Renderer */
	IDXGISwapChain*				SwapChain;
	ImGuiContext*				ImguiContext;

	/* Minhook */
	MINHOOK_CREATE				CreateHook;
	MINHOOK_REMOVE				RemoveHook;
	MINHOOK_ENABLE				EnableHook;
	MINHOOK_DISABLE				DisableHook;

	/* Logging */
	LOGGER_LOGA					Log;
	LOGGER_ADDREM				RegisterLogger;
	LOGGER_ADDREM				UnregisterLogger;

	/* Events */
	EVENTS_RAISE				RaiseEvent;
	EVENTS_SUBSCRIBE			SubscribeEvent;
	EVENTS_SUBSCRIBE			UnsubscribeEvent;

	/* Keybinds */
	KEYBINDS_REGISTER			RegisterKeybind;
	KEYBINDS_UNREGISTER			UnregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE		GetResource;
	DATALINK_SHARERESOURCE		ShareResource;

	/* Textures */
	TEXTURES_GET				GetTexture;
	TEXTURES_LOADFROMFILE		LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE	LoadTextureFromResource;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT		AddShortcut;
	QUICKACCESS_REMOVESHORTCUT  RemoveShortcut;
	QUICKACCESS_ADDSIMPLE		AddSimpleShortcut;
	QUICKACCESS_REMOVESIMPLE	RemoveSimpleShortcut;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
};

#endif