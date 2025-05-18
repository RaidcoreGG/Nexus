///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Index.h
/// Description  :  Contains an index of all paths.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef INDEX_H
#define INDEX_H

#include <Windows.h>
#include <filesystem>
#include <vector>
#include <string>

///----------------------------------------------------------------------------------------------------
/// Index Namespace
///----------------------------------------------------------------------------------------------------
namespace Index
{
	extern std::filesystem::path D_GW2;                            /* current directory -> <GW2> */
	extern std::filesystem::path D_GW2_ADDONS;                     /* <GW2>/addons */
	extern std::filesystem::path D_GW2_ADDONS_COMMON;              /* <GW2>/addons/common */
	extern std::filesystem::path D_GW2_ADDONS_COMMON_API_GW2;      /* <GW2>/addons/common/api.guildwars2.com */
	extern std::filesystem::path D_GW2_ADDONS_COMMON_API_RAIDCORE; /* <GW2>/addons/common/api.raidcore.gg */
	extern std::filesystem::path D_GW2_ADDONS_COMMON_API_GITHUB;   /* <GW2>/addons/common/api.github.com */
	extern std::filesystem::path D_GW2_ADDONS_NEXUS;               /* <GW2>/addons/Nexus */
	extern std::filesystem::path D_GW2_ADDONS_NEXUS_FONTS;         /* <GW2>/addons/Nexus/Fonts */
	extern std::filesystem::path D_GW2_ADDONS_NEXUS_LOCALES;       /* <GW2>/addons/Nexus/Locales */
	extern std::filesystem::path D_GW2_ADDONS_NEXUS_STYLES;        /* <GW2>/addons/Nexus/Styles */
	extern std::filesystem::path D_DOCUMENTS;                      /* <DOCUMENTS> */
	extern std::filesystem::path D_DOCUMENTS_GW2;                  /* <DOCUMENTS>/Guild Wars 2 */
	extern std::filesystem::path D_DOCUMENTS_GW2_INPUTBINDS;       /* <DOCUMENTS>/Guild Wars 2/InputBinds */

	extern std::filesystem::path F_HOST_DLL;
	extern std::filesystem::path F_UPDATE_DLL;
	extern std::filesystem::path F_OLD_DLL;
	extern std::filesystem::path F_SYSTEM_DLL;
	extern std::filesystem::path F_CHAINLOAD_DLL;
	extern std::filesystem::path F_ARCDPSINTEGRATION;

	extern std::filesystem::path F_LOG;
	extern std::filesystem::path F_INPUTBINDS;
	extern std::filesystem::path F_GAMEBINDS;
	extern std::filesystem::path F_GAMEBINDS_LEGACY;
	extern std::filesystem::path F_SETTINGS;
	extern std::filesystem::path F_ADDONCONFIG;
	extern std::filesystem::path F_ADDONCONFIG_DEFAULT;
	extern std::filesystem::path F_APIKEYS;

	extern std::filesystem::path F_LOCALE_EN;
	extern std::filesystem::path F_LOCALE_DE;
	extern std::filesystem::path F_LOCALE_FR;
	extern std::filesystem::path F_LOCALE_ES;

	extern std::filesystem::path F_LOCALE_BR;
	extern std::filesystem::path F_LOCALE_CZ;
	extern std::filesystem::path F_LOCALE_IT;
	extern std::filesystem::path F_LOCALE_PL;
	extern std::filesystem::path F_LOCALE_RU;
	extern std::filesystem::path F_LOCALE_CN;

	extern std::vector<std::string>	PathStore;

	///----------------------------------------------------------------------------------------------------
	/// BuildIndex:
	/// 	Initialize all path variables relative to the base modules location.
	///----------------------------------------------------------------------------------------------------
	void BuildIndex(HMODULE aBaseModule);

	///----------------------------------------------------------------------------------------------------
	/// GetGameDirectory:
	/// 	Returns the game directory.
	///----------------------------------------------------------------------------------------------------
	const char* GetGameDirectory();

	///----------------------------------------------------------------------------------------------------
	/// GetAddonDirectory:
	/// 	Returns "<game>/addons/<aName>" or "<game>/addons" if nullptr is passed.
	///----------------------------------------------------------------------------------------------------
	const char* GetAddonDirectory(const char* aName);

	///----------------------------------------------------------------------------------------------------
	/// GetCommonDirectory:
	/// 	Returns the "<game>/addons/common" directory.
	///----------------------------------------------------------------------------------------------------
	const char* GetCommonDirectory();
}

#endif
