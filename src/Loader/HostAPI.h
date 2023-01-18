#ifndef HOSTFUNCTIONTABLE_H
#define HOSTFUNCTIONTABLE_H

#include "../Logging/LogHandler.h"
#include "../Events/EventHandler.h"
#include "../Keybinds/KeybindHandler.h"
#include "../Mumble/Mumble.h"

typedef struct HostAPI
{
	/* LoggingA */
	LogASig LogTraceA;
	LogASig LogCriticalA;
	LogASig LogWarningA;
	LogASig LogInfoA;
	LogASig LogDebugA;

	/* LoggingW */
	LogWSig LogTraceW;
	LogWSig LogCriticalW;
	LogWSig LogWarningW;
	LogWSig LogInfoW;
	LogWSig LogDebugW;

	/* Events */
	RaiseEventSig RaiseEvent;
	SubscribeEventSig SubscribeEvent;

	/* Keybinds */
	RegisterKeybindSig RegisterKeybind;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
		// RESOURCE SHARING FUNCS
};

#endif