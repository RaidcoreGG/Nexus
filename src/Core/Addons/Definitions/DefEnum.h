///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DefEnum.h
/// Description  :  Enumerations for addons definitions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DEFENUM_H
#define DEFENUM_H

#include <cstdint>
#include <windows.h>

///----------------------------------------------------------------------------------------------------
/// EAddonDefFlags Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonDefFlags : uint32_t
{
	None                             = 0,
	IsVolatile                       = 1 << 0, /* is game build dependant and wants to be disabled after game updates */
	DisableHotloading                = 1 << 1, /* prevents unloading at runtime, aka. will require a restart if updated, etc. */
	OnlyLoadDuringGameLaunchSequence = 1 << 2, /* prevents loading the addon later than the initial character select */
	CanCreateImGuiContext            = 1 << 3  /* addon is capable of receiving nullptr instead of imgui context and allocators and can manage its own */
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonDefFlags);

#endif
