#ifndef LOADEDADDON_H
#define LOADEDADDON_H

#include <Windows.h>
#include <vector>

#include "AddonDefinition.h"

/* A structure that holds information about a loaded addon. */
struct ActiveAddon
{
	HMODULE				Module;
	DWORD				ModuleSize;
	AddonDefinition*	Definitions;
};

#endif