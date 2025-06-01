///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Index.cpp
/// Description  :  Path index.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Index.h"

#include <assert.h>
#include <shlobj.h>

static HMODULE               s_Module = nullptr;
static std::filesystem::path s_Paths[(int)EPath::COUNT];

void CreateIndex(HMODULE aModule)
{
	assert(!s_Module);

	/* Self DLL path. */
	char module[MAX_PATH]{};
	GetModuleFileNameA(aModule, module, MAX_PATH);
	s_Paths[(int)EPath::NexusDLL] = module;

	/* Get system directory. */
	char system[MAX_PATH]{};
	GetSystemDirectoryA(system, MAX_PATH);
	s_Paths[(int)EPath::DIR_SYSTEM] = system;

	/* Get user documents directory. */
	char documents[MAX_PATH]{};
	SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents);
	s_Paths[(int)EPath::DIR_DOCUMENTS] = documents;

	/* Get game relative directories. */
	s_Paths[(int)EPath::DIR_GW2]               = s_Paths[(int)EPath::NexusDLL].parent_path();
	s_Paths[(int)EPath::DIR_ADDONS]            = s_Paths[(int)EPath::DIR_GW2] / "addons";
	s_Paths[(int)EPath::DIR_COMMON]            = s_Paths[(int)EPath::DIR_ADDONS] / "common";
	s_Paths[(int)EPath::DIR_APICACHE_GW2]      = s_Paths[(int)EPath::DIR_COMMON] / "api.guildwars2.com";
	s_Paths[(int)EPath::DIR_APICACHE_RAIDCORE] = s_Paths[(int)EPath::DIR_COMMON] / "api.raidcore.gg";
	s_Paths[(int)EPath::DIR_APICACHE_GITHUB]   = s_Paths[(int)EPath::DIR_COMMON] / "api.github.com";
	s_Paths[(int)EPath::DIR_NEXUS]             = s_Paths[(int)EPath::DIR_ADDONS] / "Nexus";
	s_Paths[(int)EPath::DIR_TEMP]              = s_Paths[(int)EPath::DIR_NEXUS] / "Temp";
	s_Paths[(int)EPath::DIR_FONTS]             = s_Paths[(int)EPath::DIR_NEXUS] / "Fonts";
	s_Paths[(int)EPath::DIR_LOCALES]           = s_Paths[(int)EPath::DIR_NEXUS] / "Locales";
	s_Paths[(int)EPath::DIR_STYLES]            = s_Paths[(int)EPath::DIR_NEXUS] / "Styles";

	/* Get document based directories. */
	s_Paths[(int)EPath::DIR_DOCUMENTS_GW2]     = s_Paths[(int)EPath::DIR_DOCUMENTS] / "Guild Wars 2";
	s_Paths[(int)EPath::DIR_GW2_INPUTBINDS]    = s_Paths[(int)EPath::DIR_DOCUMENTS_GW2] / "InputBinds";

	/* Ensure directory tree. */
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_ADDONS]);
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_COMMON]);
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_NEXUS]);
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_TEMP]);
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_FONTS]);
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_LOCALES]);
	std::filesystem::create_directories(s_Paths[(int)EPath::DIR_STYLES]);

	/* Get files. */
	s_Paths[(int)EPath::Log]                      = s_Paths[(int)EPath::DIR_NEXUS] / "Nexus.log";
	s_Paths[(int)EPath::LastCrashLog]             = s_Paths[(int)EPath::DIR_NEXUS] / "Crash.log";
	s_Paths[(int)EPath::InputBinds]               = s_Paths[(int)EPath::DIR_NEXUS] / "InputBinds.json";
	s_Paths[(int)EPath::GameBinds]                = s_Paths[(int)EPath::DIR_NEXUS] / "GameBinds.xml";
	s_Paths[(int)EPath::Settings]                 = s_Paths[(int)EPath::DIR_NEXUS] / "Settings.json";
	s_Paths[(int)EPath::AddonConfigDefault]       = s_Paths[(int)EPath::DIR_NEXUS] / "AddonConfig.json";
	s_Paths[(int)EPath::ArcdpsIntegration]        = s_Paths[(int)EPath::DIR_NEXUS] / "arcdps_integration64.dll";
	s_Paths[(int)EPath::ThirdPartySoftwareReadme] = s_Paths[(int)EPath::DIR_NEXUS] / "THIRDPARTYSOFTWAREREADME.TXT";

	/* DLL paths. */
	s_Paths[(int)EPath::NexusDLL_Old]    = s_Paths[(int)EPath::NexusDLL].string() + ".old";
	s_Paths[(int)EPath::NexusDLL_Update] = s_Paths[(int)EPath::NexusDLL].string() + ".update";
	s_Paths[(int)EPath::D3D11]           = s_Paths[(int)EPath::DIR_SYSTEM] / "d3d11.dll";
	s_Paths[(int)EPath::D3D11Chainload]  = s_Paths[(int)EPath::DIR_GW2] / "d3d11_chainload.dll";

	/* Locales. */
	s_Paths[(int)EPath::LocaleEN] = s_Paths[(int)EPath::DIR_LOCALES] / "en_Main.json";
	s_Paths[(int)EPath::LocaleDE] = s_Paths[(int)EPath::DIR_LOCALES] / "de_Main.json";
	s_Paths[(int)EPath::LocaleFR] = s_Paths[(int)EPath::DIR_LOCALES] / "fr_Main.json";
	s_Paths[(int)EPath::LocaleES] = s_Paths[(int)EPath::DIR_LOCALES] / "es_Main.json";
	s_Paths[(int)EPath::LocaleBR] = s_Paths[(int)EPath::DIR_LOCALES] / "br_Main.json";
	s_Paths[(int)EPath::LocaleCZ] = s_Paths[(int)EPath::DIR_LOCALES] / "cz_Main.json";
	s_Paths[(int)EPath::LocaleIT] = s_Paths[(int)EPath::DIR_LOCALES] / "it_Main.json";
	s_Paths[(int)EPath::LocalePL] = s_Paths[(int)EPath::DIR_LOCALES] / "pl_Main.json";
	s_Paths[(int)EPath::LocaleRU] = s_Paths[(int)EPath::DIR_LOCALES] / "ru_Main.json";
	s_Paths[(int)EPath::LocaleCN] = s_Paths[(int)EPath::DIR_LOCALES] / "cn_Main.json";

	/* Set the module. */
	s_Module = aModule;
}

std::filesystem::path Index(EPath aIndex)
{
	assert(s_Module);

	return s_Paths[(int)aIndex];
}
