#ifndef EADDONFLAGS_H
#define EADDONFLAGS_H

#include <windows.h>

enum class EAddonFlags
{
	None                             = 0,
	IsVolatile                       = 1 << 0, /* is game build dependant and wants to be disabled after game updates */
	DisableHotloading                = 1 << 1, /* prevents unloading at runtime, aka. will require a restart if updated, etc. */
	OnlyLoadDuringGameLaunchSequence = 1 << 2, /* prevents loading the addon later than the initial character select */
	CanCreateImGuiContext            = 1 << 3  /* addon is capable of receiving nullptr instead of imgui context and allocators and can manage its own */
};

DEFINE_ENUM_FLAG_OPERATORS(EAddonFlags);

#endif
