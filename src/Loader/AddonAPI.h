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
	/* LoggingA */
	LOGGER_LOGA			LogTraceA;
	LOGGER_LOGA			LogCriticalA;
	LOGGER_LOGA			LogWarningA;
	LOGGER_LOGA			LogInfoA;
	LOGGER_LOGA			LogDebugA;

	/* LoggingW */
	LOGGER_LOGW			LogTraceW;
	LOGGER_LOGW			LogCriticalW;
	LOGGER_LOGW			LogWarningW;
	LOGGER_LOGW			LogInfoW;
	LOGGER_LOGW			LogDebugW;

	/* Logging API */
	LOGGER_ADDREM		RegisterLogger;
	LOGGER_ADDREM		UnregisterLogger;
};

struct AddonAPI
{
	IDXGISwapChain*		SwapChain;
	ImGuiContext*		ImguiContext;
	LinkedMem*			MumbleLink;

	VTableMinhook		MinhookFunctions;
	VTableLogging		LoggingFunctions;

	/* Events */
	EVENTS_RAISE		RaiseEvent;
	EVENTS_SUBSCRIBE	SubscribeEvent;

	/* Keybinds */
	KEYBINDS_REGISTER	RegisterKeybind;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
		// RESOURCE SHARING FUNCS
};

#endif