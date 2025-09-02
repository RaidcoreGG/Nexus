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
	None                  = 0,
	IsVolatile            = 1 << 0, /* Makes the addon automatically disable, if the game updated. */
	DisableHotloading     = 1 << 1, /* Prevents the addon from being unloaded at runtime. Unload will still be called on shutdown, if defined. */
	LaunchOnly            = 1 << 2, /* Prevents the addon from getting loaded at runtime after the initial game launch. */
	CanCreateImGuiContext = 1 << 3  /* Addon is capable of receiving nullptr instead of imgui context and allocators and can manage its own. (User pref.) */
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonDefFlags);

#endif
