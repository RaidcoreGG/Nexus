#ifndef LOADEDADDON_H
#define LOADEDADDON_H

#include <Windows.h>

#include "AddonDefinition.h"

struct LoadedAddon
{
	HMODULE				Module;
	DWORD				ModuleSize;
	AddonDefinition*	Definitions;
};

#endif