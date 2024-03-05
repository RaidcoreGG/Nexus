#ifndef ADDON_H
#define ADDON_H

#include <Windows.h>
#include <vector>

#include "EAddonState.h"
#include "AddonDefinition.h"

/* A structure that holds information about a loaded addon. */
struct Addon
{
	EAddonState					State;
	HMODULE						Module;
	DWORD						ModuleSize;
	std::vector<unsigned char>	MD5;
	AddonDefinition*			Definitions;
	bool						ShouldDisableNextLaunch;
	bool						IsPausingUpdates;
	bool						WillBeUninstalled;
	bool						IsDisabledUntilUpdate;
};

#endif