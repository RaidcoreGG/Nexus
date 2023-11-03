#ifndef PATHS_H
#define PATHS_H

#include <Windows.h>

#include "core.h"

/* Namespace for global path variables */
namespace Path
{
	extern char D_GW2							[];
	extern char D_GW2_ADDONS					[];
	extern char D_GW2_ADDONS_COMMON				[];
	extern char D_GW2_ADDONS_COMMON_API_V2		[];
	extern char D_GW2_ADDONS_RAIDCORE			[];
	extern char D_GW2_ADDONS_RAIDCORE_LOCALES	[];

	extern char F_HOST_DLL						[];
	extern char F_TEMP_DLL						[];
	extern char F_SYSTEM_DLL					[];
	extern char F_CHAINLOAD_DLL					[];

	extern char F_LOG							[];
	extern char F_KEYBINDS						[];
	extern char F_FONT							[];
	extern char F_SETTINGS						[];
	extern char F_APIKEYS						[];

	/* Initialize all path variables relative to the base modules location */
	void Initialize(HMODULE aBaseModule);

	const char* GetGameDirectory();
	const char* GetAddonDirectory(const char* aName);
	const char* GetCommonDirectory();
}

#endif