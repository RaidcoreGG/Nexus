#ifndef EADDONSTATE_H
#define EADDONSTATE_H

enum class EAddonState
{
	None,

	NotLoaded,					/* Addon is not loaded into process. */
	NotLoadedIncompatible,		/* The file is incompatible with Nexus. */
	NotLoadedIncompatibleAPI,	/* Addon requested an API that doesn't exist. */
	NotLoadedDuplicate,			/* Another addon with that signature is already loaded. */

	Ready,						/* Addon is loaded into process, ready to call Addon::Load(). */

	Loaded,						/* Addon is loaded into process and Addon::Load was called. */
	LoadedLOCKED				/* Addon is loaded, but locked and mustn't be unloaded. */
};

#endif