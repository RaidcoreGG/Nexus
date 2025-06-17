#ifndef ADDON_H
#define ADDON_H

#include <Windows.h>
#include <vector>
#include <filesystem>

#include "LdrEnum.h"
#include "Core/Addons/Definitions/AddonDefV1.h"

/* A structure that holds information about a loaded addon. */
struct Addon_t
{
	EAddonState					State;
	std::filesystem::path		Path;
	std::vector<unsigned char>	MD5;
	HMODULE						Module;
	DWORD						ModuleSize;
	AddonDef_t*			Definitions;

	/* Saved states */
	bool						IsPausingUpdates;
	bool						IsDisabledUntilUpdate;
	bool						AllowPrereleases;
	bool						IsFavorite;

	/* Runtime states */
	signed int					MatchSignature;
	//bool						IsInitialLoad = true;
	bool						IsCheckingForUpdates;
	bool						IsWaitingForUnload;
	bool						IsFlaggedForUninstall;
	bool						IsFlaggedForDisable;
	bool						IsFlaggedForEnable;
};

#endif