#ifndef EADDONFLAGS_H
#define EADDONFLAGS_H

enum class EAddonFlags
{
	None								= 0,
	IsVolatile							= 1 << 0,	/* is hooking functions or doing anything else that's volatile and game build dependant */
	DisableHotloading					= 1 << 1,	/* prevents unloading at runtime, aka. will require a restart if updated, etc. */
	OnlyLoadDuringGameLaunchSequence	= 1 << 2,	/* prevents loading later than character select, aka will require restart to get loaded */
	SyncUnload							= 1 << 3	/* unloading the addon will be synchronous, rather than async and will stall the render thread */
};

EAddonFlags operator|(EAddonFlags lhs, EAddonFlags rhs);

EAddonFlags operator&(EAddonFlags lhs, EAddonFlags rhs);
#endif