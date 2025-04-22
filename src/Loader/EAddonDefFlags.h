///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EAddonDefFlags.h
/// Description  :  Contains the addon definition flags.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EADDONDEFFLAGS_H
#define EADDONDEFFLAGS_H

///----------------------------------------------------------------------------------------------------
/// EAddonDefFlags Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonDefFlags
{
	None                 = 0,
	IsVolatile           = 1 << 0, /* Is hooking functions or doing anything else that's volatile and game build dependant. */
	DisableHotloading    = 1 << 1, /* Prevents unloading at runtime, aka. will require a restart if updated, etc. */
	OnlyLoadOnGameLaunch = 1 << 2  /* Prevents loading later than character select, aka will require restart to get loaded. */
};

DEFINE_ENUM_FLAG_OPERATORS(EAddonDefFlags)

#endif
