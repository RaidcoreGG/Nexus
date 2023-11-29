#ifndef ADDON_H
#define ADDON_H

#include <Windows.h>

#include "EAddonState.h"
#include "AddonDefinition.h"

/* A structure that holds information about a loaded addon. */
struct Addon
{
	EAddonState			State;
	HMODULE				Module;
	DWORD				ModuleSize;
	AddonDefinition		Definitions;
};

#endif