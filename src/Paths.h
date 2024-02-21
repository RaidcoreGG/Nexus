#ifndef PATHS_H
#define PATHS_H

#include <Windows.h>
#include <filesystem>
#include <vector>
#include <string>

/* Namespace for global path variables */
namespace Path
{
	extern std::filesystem::path D_GW2;
	extern std::filesystem::path D_GW2_ADDONS;
	extern std::filesystem::path D_GW2_ADDONS_COMMON;
	extern std::filesystem::path D_GW2_ADDONS_COMMON_API_GW2;
	extern std::filesystem::path D_GW2_ADDONS_COMMON_API_RAIDCORE;
	extern std::filesystem::path D_GW2_ADDONS_COMMON_API_GITHUB;
	extern std::filesystem::path D_GW2_ADDONS_NEXUS;
	extern std::filesystem::path D_GW2_ADDONS_RAIDCORE_LOCALES;

	extern std::filesystem::path F_HOST_DLL;
	extern std::filesystem::path F_UPDATE_DLL;
	extern std::filesystem::path F_OLD_DLL;
	extern std::filesystem::path F_SYSTEM_DLL;
	extern std::filesystem::path F_CHAINLOAD_DLL;
	extern std::filesystem::path F_ARCDPSINTEGRATION;

	extern std::filesystem::path F_LOG;
	extern std::filesystem::path F_KEYBINDS;
	extern std::filesystem::path F_FONT;
	extern std::filesystem::path F_SETTINGS;
	extern std::filesystem::path F_APIKEYS;

	extern std::vector<std::string>	ExistingPaths;

	/* Initialize all path variables relative to the base modules location */
	void Initialize(HMODULE aBaseModule);

	const char* GetGameDirectory();
	const char* GetAddonDirectory(const char* aName);
	const char* GetCommonDirectory();
}

#endif