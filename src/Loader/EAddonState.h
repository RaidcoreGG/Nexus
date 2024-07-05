#ifndef EADDONSTATE_H
#define EADDONSTATE_H

#include <sal.h>

enum class EAddonState
{
	None,

	NotLoaded,						/* Addon is not loaded. */
	NotLoadedDuplicate,				/* Addon is not loaded, because it has the same signature as another addon. */
	NotLoadedIncompatible,			/* The file is incompatible with Nexus. */
	NotLoadedIncompatibleAPI,		/* Addon requested an API that doesn't exist. */

	Loaded,							/* Addon is loaded. */
	LoadedLOCKED					/* Addon is loaded, but locked and mustn't be unloaded. */
};

/// returns null if value is not in enum
_Ret_maybenull_z_ char const* ToString(EAddonState state);

#endif
