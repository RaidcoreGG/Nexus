#ifndef ADDONAPI_H
#define ADDONAPI_H

#include <dxgi.h>
#include "../Mumble/LinkedMem.h"
#include "../Logging/LogHandler.h"
#include "../Events/EventHandler.h"
#include "../Keybinds/KeybindHandler.h"
#include "../imgui/imgui.h"
#include "../minhook/mh_hook.h"
#include "../DataLink/DataLink.h"

using namespace Mumble;

struct AddonAPI
{
	/* Renderer */
	IDXGISwapChain*			SwapChain;
	ImGuiContext*			ImguiContext;
	unsigned*				WindowWidth;
	unsigned*				WindowHeight;

	/* Minhook */
	MINHOOK_CREATE			CreateHook;
	MINHOOK_REMOVE			RemoveHook;
	MINHOOK_ENABLE			EnableHook;
	MINHOOK_DISABLE			DisableHook;

	/* Logging */
	LOGGER_LOGA				Log;
	LOGGER_ADDREM			RegisterLogger;
	LOGGER_ADDREM			UnregisterLogger;

	/* Events */
	EVENTS_RAISE			RaiseEvent;
	EVENTS_SUBSCRIBE		SubscribeEvent;
	EVENTS_SUBSCRIBE		UnsubscribeEvent;

	/* Keybinds */
	KEYBINDS_REGISTER		RegisterKeybind;
	KEYBINDS_UNREGISTER		UnregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE	GetResource;
	DATALINK_SHARERESOURCE	ShareResource;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
};

#endif