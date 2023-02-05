#ifndef HOSTFUNCTIONTABLE_H
#define HOSTFUNCTIONTABLE_H

#include <dxgi.h>
#include "../Mumble/LinkedMem.h"
#include "../Logging/LogHandler.h"
#include "../Events/EventHandler.h"
#include "../Keybinds/KeybindHandler.h"
#include "../imgui/imgui.h"
#include "../minhook/mh_hook.h"

using namespace Mumble;

struct VTableMinhook
{
	MINHOOK_CREATE		CreateHook;
	MINHOOK_REMOVE		RemoveHook;
	MINHOOK_ENABLE		EnableHook;
	MINHOOK_DISABLE		DisableHook;
};

struct VTableLogging
{
	LOGGER_LOGA			Log;
	LOGGER_ADDREM		RegisterLogger;
	LOGGER_ADDREM		UnregisterLogger;
};

struct AddonAPI
{
	IDXGISwapChain*		SwapChain;
	ImGuiContext*		ImguiContext;
	LinkedMem*			MumbleLink;
	unsigned*			WindowWidth;
	unsigned*			WindowHeight;

	VTableMinhook		MinhookFunctions;
	VTableLogging		LoggingFunctions;

	/* Events */
	EVENTS_RAISE		RaiseEvent;
	EVENTS_SUBSCRIBE	SubscribeEvent;
	EVENTS_SUBSCRIBE	UnsubscribeEvent;

	/* Keybinds */
	KEYBINDS_REGISTER	RegisterKeybind;
	KEYBINDS_UNREGISTER	UnregisterKeybind;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
		// RESOURCE SHARING FUNCS
};

#endif