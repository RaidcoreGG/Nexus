#ifndef HOSTFUNCTIONTABLE_H
#define HOSTFUNCTIONTABLE_H

#include "../Logging/LogHandler.h"
#include "../Events/EventHandler.h"
#include "../Keybinds/KeybindHandler.h"
#include "../Mumble/Mumble.h"

typedef struct HostFunctionTable
{
	/* LoggingA */
	LogASig LogTrace;
	LogASig LogCritical;
	LogASig LogWarning;
	LogASig LogInfo;
	LogASig LogDebug;

	/* LoggingW */
	LogWSig LogTrace;
	LogWSig LogCritical;
	LogWSig LogWarning;
	LogWSig LogInfo;
	LogWSig LogDebug;

	/* Events */
	RaiseEventSig RaiseEvent;
	SubscribeEventSig SubscribeEvent;

	/* Keybinds */
		// RegisterKeybindSig RegisterKeybind;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
		// RESOURCE SHARING FUNCS
};

#endif