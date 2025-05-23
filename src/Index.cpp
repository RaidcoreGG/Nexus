///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Index.cpp
/// Description  :  Contains an index of all paths.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Index.h"

#include <shlobj.h>

#include "Util/Paths.h"

namespace Index
{
	std::filesystem::path D_GW2{};
	std::filesystem::path D_GW2_ADDONS{};
	std::filesystem::path D_GW2_ADDONS_COMMON{};
	std::filesystem::path D_GW2_ADDONS_COMMON_API_GW2{};
	std::filesystem::path D_GW2_ADDONS_COMMON_API_RAIDCORE{};
	std::filesystem::path D_GW2_ADDONS_COMMON_API_GITHUB{};
	std::filesystem::path D_GW2_ADDONS_NEXUS{};
	std::filesystem::path D_GW2_ADDONS_NEXUS_FONTS{};
	std::filesystem::path D_GW2_ADDONS_NEXUS_LOCALES{};
	std::filesystem::path D_GW2_ADDONS_NEXUS_STYLES{};
	std::filesystem::path D_DOCUMENTS{};
	std::filesystem::path D_DOCUMENTS_GW2{};
	std::filesystem::path D_DOCUMENTS_GW2_INPUTBINDS{};

	std::filesystem::path F_HOST_DLL{};
	std::filesystem::path F_UPDATE_DLL{};
	std::filesystem::path F_OLD_DLL{};
	std::filesystem::path F_SYSTEM_DLL{};
	std::filesystem::path F_CHAINLOAD_DLL{};
	std::filesystem::path F_ARCDPSINTEGRATION{};

	std::filesystem::path F_LOG{};
	std::filesystem::path F_INPUTBINDS{};
	std::filesystem::path F_GAMEBINDS{};
	std::filesystem::path F_GAMEBINDS_LEGACY{};
	std::filesystem::path F_SETTINGS{};
	std::filesystem::path F_ADDONCONFIG{};
	std::filesystem::path F_ADDONCONFIG_DEFAULT{};
	std::filesystem::path F_APIKEYS{};

	std::filesystem::path F_THIRDPARTYNOTICES;

	std::filesystem::path F_LOCALE_EN{};
	std::filesystem::path F_LOCALE_DE{};
	std::filesystem::path F_LOCALE_FR{};
	std::filesystem::path F_LOCALE_ES{};

	std::filesystem::path F_LOCALE_BR{};
	std::filesystem::path F_LOCALE_CZ{};
	std::filesystem::path F_LOCALE_IT{};
	std::filesystem::path F_LOCALE_PL{};
	std::filesystem::path F_LOCALE_RU{};
	std::filesystem::path F_LOCALE_CN{};

	std::vector<std::string> PathStore;

	void BuildIndex(HMODULE aBaseModule)
	{
		F_HOST_DLL = Path::GetModule(aBaseModule); /* get self dll path */

		/* directories */
		D_GW2                            = F_HOST_DLL.parent_path();
		D_GW2_ADDONS                     = D_GW2 / "addons";
		D_GW2_ADDONS_COMMON              = D_GW2_ADDONS / "common";
		D_GW2_ADDONS_COMMON_API_GW2      = D_GW2_ADDONS_COMMON / "api.guildwars2.com";
		D_GW2_ADDONS_COMMON_API_RAIDCORE = D_GW2_ADDONS_COMMON / "api.raidcore.gg";
		D_GW2_ADDONS_COMMON_API_GITHUB   = D_GW2_ADDONS_COMMON / "api.github.com";
		D_GW2_ADDONS_NEXUS               = D_GW2_ADDONS / "Nexus";
		D_GW2_ADDONS_NEXUS_FONTS         = D_GW2_ADDONS_NEXUS / "Fonts";
		D_GW2_ADDONS_NEXUS_LOCALES       = D_GW2_ADDONS_NEXUS / "Locales";
		D_GW2_ADDONS_NEXUS_STYLES        = D_GW2_ADDONS_NEXUS / "Styles";
		char documents[MAX_PATH];
		SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents);
		D_DOCUMENTS = documents;
		D_DOCUMENTS_GW2 = D_DOCUMENTS / "Guild Wars 2";
		D_DOCUMENTS_GW2_INPUTBINDS = D_DOCUMENTS_GW2 / "InputBinds";

		/* ensure directory tree*/
		Path::CreateDir(D_GW2_ADDONS);
		Path::CreateDir(D_GW2_ADDONS_COMMON);
		Path::CreateDir(D_GW2_ADDONS_NEXUS);
		Path::CreateDir(D_GW2_ADDONS_NEXUS_FONTS);
		Path::CreateDir(D_GW2_ADDONS_NEXUS_LOCALES);
		Path::CreateDir(D_GW2_ADDONS_NEXUS_STYLES);

		/* files */
		F_LOG = D_GW2_ADDONS_NEXUS / "Nexus.log";
		F_INPUTBINDS = D_GW2_ADDONS_NEXUS / "InputBinds.json";
		F_GAMEBINDS = D_GW2_ADDONS_NEXUS / "GameBinds.xml";
		F_GAMEBINDS_LEGACY = D_GW2_ADDONS_NEXUS / "GameBinds.json";
		F_SETTINGS = D_GW2_ADDONS_NEXUS / "Settings.json";
		F_ADDONCONFIG = D_GW2_ADDONS_NEXUS / "AddonConfig.json";
		F_ADDONCONFIG_DEFAULT = D_GW2_ADDONS_NEXUS / "AddonConfig.json";
		F_APIKEYS = D_GW2_ADDONS_COMMON / "APIKeys.json";

		F_THIRDPARTYNOTICES = D_GW2_ADDONS_NEXUS / "THIRDPARTYSOFTWAREREADME.TXT";
			
		/* static paths */
		F_OLD_DLL = F_HOST_DLL.string() + ".old";
		F_UPDATE_DLL = F_HOST_DLL.string() + ".update";
		F_SYSTEM_DLL = Path::GetSystem("d3d11.dll");
		F_CHAINLOAD_DLL = D_GW2 / "d3d11_chainload.dll";
		F_ARCDPSINTEGRATION = D_GW2_ADDONS_NEXUS / "arcdps_integration64.dll";

		F_LOCALE_EN = D_GW2_ADDONS_NEXUS_LOCALES / "en_Main.json";
		F_LOCALE_DE = D_GW2_ADDONS_NEXUS_LOCALES / "de_Main.json";
		F_LOCALE_FR = D_GW2_ADDONS_NEXUS_LOCALES / "fr_Main.json";
		F_LOCALE_ES = D_GW2_ADDONS_NEXUS_LOCALES / "es_Main.json";

		F_LOCALE_BR = D_GW2_ADDONS_NEXUS_LOCALES / "br_Main.json";
		F_LOCALE_CZ = D_GW2_ADDONS_NEXUS_LOCALES / "cz_Main.json";
		F_LOCALE_IT = D_GW2_ADDONS_NEXUS_LOCALES / "it_Main.json";
		F_LOCALE_PL = D_GW2_ADDONS_NEXUS_LOCALES / "pl_Main.json";
		F_LOCALE_RU = D_GW2_ADDONS_NEXUS_LOCALES / "ru_Main.json";
		F_LOCALE_CN = D_GW2_ADDONS_NEXUS_LOCALES / "cn_Main.json";

		/* push to paths */
		PathStore.push_back(D_GW2.string());
		PathStore.push_back(D_GW2_ADDONS_COMMON.string());
		PathStore.push_back(D_GW2_ADDONS_COMMON_API_GW2.string());
		PathStore.push_back(D_GW2_ADDONS_COMMON_API_RAIDCORE.string());

		/* migrate old font */
		std::filesystem::path legacyFontPath = D_GW2_ADDONS_NEXUS / "Font.ttf";
		if (std::filesystem::exists(legacyFontPath))
		{
			std::filesystem::path targetPath = D_GW2_ADDONS_NEXUS_FONTS / legacyFontPath.filename();

			targetPath = Path::GetUnused(targetPath);

			try
			{
				std::filesystem::rename(legacyFontPath, targetPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				/* do nothing. we cannot log here yet, but this is also not really relevant. */
			}
		}

		/* migrate keybinds */

		std::filesystem::path legacyKeybindsPath = D_GW2_ADDONS_NEXUS / "Keybinds.json";
		if (std::filesystem::exists(legacyKeybindsPath))
		{
			std::filesystem::path targetPath = F_INPUTBINDS;

			targetPath = Path::GetUnused(targetPath);

			try
			{
				std::filesystem::rename(legacyKeybindsPath, targetPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				/* do nothing. we cannot log here yet, but this is also not really relevant. */
			}
		}
	}

	const char* GetGameDirectory()
	{
		// guaranteed to exist from init func
		const auto& it = std::find(PathStore.begin(), PathStore.end(), D_GW2.string());
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

		const auto& it = std::find(PathStore.begin(), PathStore.end(), str);

		if (it == PathStore.end())
		{
			PathStore.push_back(str);
			return PathStore.back().c_str();
		}

		return it->c_str();
	}

	const char* GetCommonDirectory()
	{
		// guaranteed to exist from init func
		const auto& it = std::find(PathStore.begin(), PathStore.end(), D_GW2_ADDONS_COMMON.string());
		return it->c_str();
	}
}
