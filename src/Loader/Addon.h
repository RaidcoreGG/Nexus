#ifndef ADDON_H
#define ADDON_H

#include <Windows.h>
#include <vector>
#include <filesystem>

#include "EAddonState.h"
#include "AddonDefinition.h"

/* A structure that holds information about a loaded addon. */
struct Addon
{
	EAddonState					State;
	std::filesystem::path		Path;
	std::vector<unsigned char>	MD5;
	HMODULE						Module;
	DWORD						ModuleSize;
	AddonDefinition*			Definitions;

	/* Saved states */
	bool						IsPausingUpdates;
	bool						IsDisabledUntilUpdate;

	/* Runtime states */
	signed int					MatchSignature;
	//bool						IsInitialLoad = true;
	bool						IsCheckingForUpdates;
	bool						IsWaitingForUnload;
	bool						IsFlaggedForUninstall;
	bool						IsFlaggedForDisable;
};

#endif