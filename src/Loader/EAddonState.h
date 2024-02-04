#ifndef EADDONSTATE_H
#define EADDONSTATE_H

enum class EAddonState
{
	None,
	
	Loaded,					/* Addon is loaded. */
	LoadedLOCKED,			/* Addon is loaded, but locked and mustn't be unloaded. */

	NotLoaded,				/* Addon is not loaded. */
	NotLoadedDuplicate,		/* Addon is not loaded, because it has the same signature as another addon. */

	Incompatible,			/* The file is incompatible with Nexus. */
	IncompatibleAPI			/* Addon requested an API that doesn't exist. */
};

#endif