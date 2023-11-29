#ifndef EADDONSTATE_H
#define EADDONSTATE_H

enum class EAddonState
{
	None,
	
	Loaded,					/* Addon is loaded. */

	NotLoaded,				/* Addon is not loaded. */
	NotLoadedDuplicate,		/* Addon is not loaded, because it has the same signature as another addon. */

	Incompatible,			/* The file is incompatible with Nexus. */
	IncompatibleAPI,		/* Addon requested an API that doesn't exist. */

	Reload,					/* Addon is being reloaded due to an update. */
};

#endif