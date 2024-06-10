#include "Paths.h"

#include <algorithm>

#include "core.h"

namespace Path
{
	std::filesystem::path D_GW2{};
	std::filesystem::path D_GW2_ADDONS{};
	std::filesystem::path D_GW2_ADDONS_COMMON{};
	std::filesystem::path D_GW2_ADDONS_COMMON_API_GW2{};
	std::filesystem::path D_GW2_ADDONS_COMMON_API_RAIDCORE{};
	std::filesystem::path D_GW2_ADDONS_COMMON_API_GITHUB{};
	std::filesystem::path D_GW2_ADDONS_NEXUS{};
	std::filesystem::path D_GW2_ADDONS_RAIDCORE_LOCALES{};

	std::filesystem::path F_HOST_DLL{};
	std::filesystem::path F_UPDATE_DLL{};
	std::filesystem::path F_OLD_DLL{};
	std::filesystem::path F_SYSTEM_DLL{};
	std::filesystem::path F_CHAINLOAD_DLL{};
	std::filesystem::path F_ARCDPSINTEGRATION{};

	std::filesystem::path F_LOG{};
	std::filesystem::path F_KEYBINDS{};
	std::filesystem::path F_FONT{};
	std::filesystem::path F_SETTINGS{};
	std::filesystem::path F_ADDONCONFIG{};
	std::filesystem::path F_APIKEYS{};

	std::filesystem::path F_LOCALE_EN{};
	std::filesystem::path F_LOCALE_DE{};
	std::filesystem::path F_LOCALE_FR{};
	std::filesystem::path F_LOCALE_ES{};

	std::filesystem::path F_LOCALE_BR{};
	std::filesystem::path F_LOCALE_CZ{};
	std::filesystem::path F_LOCALE_IT{};
	std::filesystem::path F_LOCALE_PL{};
	std::filesystem::path F_LOCALE_RU{};

	std::vector<std::string> ExistingPaths;

	void Initialize(HMODULE aBaseModule)
	{
		char buff[MAX_PATH]{};
		GetModuleFileNameA(aBaseModule, buff, MAX_PATH);								/* get self dll path */
		F_HOST_DLL = buff;

		/* directories */
		D_GW2 = F_HOST_DLL.parent_path();												/* get current directory */
		D_GW2_ADDONS = D_GW2 / "addons";												/* get addons path */
		D_GW2_ADDONS_COMMON = D_GW2_ADDONS / "common";									/* get addons/common path */
		D_GW2_ADDONS_COMMON_API_GW2 = D_GW2_ADDONS_COMMON / "api.guildwars2.com";		/* get addons/common/api.guildwars2.com path */
		D_GW2_ADDONS_COMMON_API_RAIDCORE = D_GW2_ADDONS_COMMON / "api.raidcore.gg";		/* get addons/common/api.raidcore.gg path */
		D_GW2_ADDONS_COMMON_API_GITHUB = D_GW2_ADDONS_COMMON / "api.github.com";		/* get addons/common/api.github.com path */
		D_GW2_ADDONS_NEXUS = D_GW2_ADDONS / "Nexus";									/* get addons/Nexus path */
		D_GW2_ADDONS_RAIDCORE_LOCALES = D_GW2_ADDONS_NEXUS / "Locales";					/* get addons/Nexus/Locales path */

		/* ensure folder tree*/
		std::filesystem::create_directory(D_GW2_ADDONS);								/* ensure addons dir */
		std::filesystem::create_directory(D_GW2_ADDONS_COMMON);							/* ensure addons/common dir */
		std::filesystem::create_directory(D_GW2_ADDONS_NEXUS);							/* ensure addons/Nexus dir */
		std::filesystem::create_directory(D_GW2_ADDONS_RAIDCORE_LOCALES);				/* ensure addons/Nexus/Locales dir */	

		/* files */
		F_LOG = D_GW2_ADDONS_NEXUS / "Nexus.log";										/* get log path */
		F_KEYBINDS = D_GW2_ADDONS_NEXUS / "Keybinds.json";								/* get keybinds path */
		F_FONT = D_GW2_ADDONS_NEXUS / "Font.ttf";										/* get font path */
		F_SETTINGS = D_GW2_ADDONS_NEXUS / "Settings.json";								/* get settings path */
		F_ADDONCONFIG = D_GW2_ADDONS_NEXUS / "AddonConfig.json";						/* get addon config path */
		F_APIKEYS = D_GW2_ADDONS_COMMON / "APIKeys.json";								/* get apikeys path */
			
		/* static paths */
		F_OLD_DLL = F_HOST_DLL.string() + ".old";										/* get old dll path */
		F_UPDATE_DLL = F_HOST_DLL.string() + ".update";									/* get update dll path */
		PathSystemAppend(F_SYSTEM_DLL, "d3d11.dll");									/* get system dll path */
		F_CHAINLOAD_DLL = D_GW2 / "d3d11_chainload.dll";								/* get chainload dll path */
		F_ARCDPSINTEGRATION = D_GW2_ADDONS_NEXUS / "arcdps_integration64.dll";			/* get arcdps integration dll path */

		F_LOCALE_EN = D_GW2_ADDONS_RAIDCORE_LOCALES / "en_Main.json";
		F_LOCALE_DE = D_GW2_ADDONS_RAIDCORE_LOCALES / "de_Main.json";
		F_LOCALE_FR = D_GW2_ADDONS_RAIDCORE_LOCALES / "fr_Main.json";
		F_LOCALE_ES = D_GW2_ADDONS_RAIDCORE_LOCALES / "es_Main.json";

		F_LOCALE_BR = D_GW2_ADDONS_RAIDCORE_LOCALES / "br_Main.json";
		F_LOCALE_CZ = D_GW2_ADDONS_RAIDCORE_LOCALES / "cz_Main.json";
		F_LOCALE_IT = D_GW2_ADDONS_RAIDCORE_LOCALES / "it_Main.json";
		F_LOCALE_PL = D_GW2_ADDONS_RAIDCORE_LOCALES / "pl_Main.json";
		F_LOCALE_RU = D_GW2_ADDONS_RAIDCORE_LOCALES / "ru_Main.json";

		/* push to paths */
		ExistingPaths.push_back(D_GW2.string());
		ExistingPaths.push_back(D_GW2_ADDONS_COMMON.string());
		ExistingPaths.push_back(D_GW2_ADDONS_COMMON_API_GW2.string());
		ExistingPaths.push_back(D_GW2_ADDONS_COMMON_API_RAIDCORE.string());
	}

	const char* GetGameDirectory()
	{
		// guaranteed to exist from init func
		const auto& it = std::find(ExistingPaths.begin(), ExistingPaths.end(), D_GW2.string());
		return it->c_str();
	}

	const char* GetAddonDirectory(const char* aName)
	{
		std::string str;

		if (aName == nullptr || strcmp(aName, "") == 0)
		{
			str = D_GW2_ADDONS.string();
		}
		else
		{
			str = (D_GW2_ADDONS / aName).string();
		}

		const auto& it = std::find(ExistingPaths.begin(), ExistingPaths.end(), str);

		if (it == ExistingPaths.end())
		{
			ExistingPaths.push_back(str);
			return ExistingPaths.back().c_str();
		}

		return it->c_str();
	}

	const char* GetCommonDirectory()
	{
		// guaranteed to exist from init func
		const auto& it = std::find(ExistingPaths.begin(), ExistingPaths.end(), D_GW2_ADDONS_COMMON.string());
		return it->c_str();
	}
}