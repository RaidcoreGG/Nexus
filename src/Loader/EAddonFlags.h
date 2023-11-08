#ifndef EADDONFLAGS_H
#define EADDONFLAGS_H

enum class EAddonFlags
{
	None = 0,
	IsVolatile = 1,				/* is hooking functions or doing anything else that's volatile and game build dependant */
	DisableHotloading = 2		/* prevents unloading at runtime, aka. will require a restart if updated, etc. */
};

EAddonFlags operator|(EAddonFlags lhs, EAddonFlags rhs);

EAddonFlags operator&(EAddonFlags lhs, EAddonFlags rhs);
#endif