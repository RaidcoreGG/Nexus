///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EAddonFlags.h
/// Description  :  Contains the addon definition flags.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EADDONFLAGS_H
#define EADDONFLAGS_H

///----------------------------------------------------------------------------------------------------
/// EAddonFlags Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonFlags
{
	None                 = 0,
	IsVolatile           = 1 << 0, /* Is hooking functions or doing anything else that's volatile and game build dependant. */
	DisableHotloading    = 1 << 1, /* Prevents unloading at runtime, aka. will require a restart if updated, etc. */
	OnlyLoadOnGameLaunch = 1 << 2, /* Prevents loading later than character select, aka will require restart to get loaded. */
	SyncUnload           = 1 << 3  /* Unloading the addon will be synchronous, rather than async and will stall the render thread. */
};

#endif
