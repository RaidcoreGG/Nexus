#ifndef EADDONSTATE_H
#define EADDONSTATE_H

enum class EAddonState
{
	None,

	NotLoaded,						/* Addon_t is not loaded. */
	NotLoadedDuplicate,				/* Addon_t is not loaded, because it has the same signature as another addon. */
	NotLoadedIncompatible,			/* The file is incompatible with Nexus. */
	NotLoadedIncompatibleAPI,		/* Addon_t requested an API that doesn't exist. */

	Loaded,							/* Addon_t is loaded. */
	LoadedLOCKED					/* Addon_t is loaded, but locked and mustn't be unloaded. */
};

#endif
