///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DefEnum.h
/// Description  :  Enumerations for addons definitions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <windows.h>

///----------------------------------------------------------------------------------------------------
/// EAddonDefFlags Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonDefFlags : uint32_t
{
	None                  = 0,
	IsVolatile            = 1 << 0, /* Makes the addon automatically disable, if the game updated.                                                        */
	DisableHotloading     = 1 << 1, /* Prevents the addon from being unloaded at runtime. Unload will still be called on shutdown, if defined.            */
	LaunchOnly            = 1 << 2, /* Prevents the addon from getting loaded at runtime after the initial game launch.                                   */
	CanCreateImGuiContext = 1 << 3, /* Addon is capable of receiving nullptr instead of imgui context and allocators and can manage its own. (User pref.) */
	ForceUpdate           = 1 << 4  /* Addon should always be kept up-to-date. E.g. to avoid exploiting old versions.                                     */
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonDefFlags);
