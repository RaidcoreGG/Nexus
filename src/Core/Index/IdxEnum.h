///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IdxEnum.h
/// Description  :  Enumerations for the Index.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IDXENUM_H
#define IDXENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EPath Enumeration
///----------------------------------------------------------------------------------------------------
enum class EPath : uint32_t
{
	DIR_SYSTEM,               /* <SYSTEM32>                                      */

	DIR_GW2,                  /* <GW2>                                           */
	DIR_ADDONS,               /* <GW2>/addons                                    */
	DIR_COMMON,               /* <GW2>/addons/common                             */
	DIR_APICACHE_GW2,         /* <GW2>/addons/common/api.guildwars2.com          */
	DIR_APICACHE_RAIDCORE,    /* <GW2>/addons/common/api.raidcore.gg             */
	DIR_APICACHE_GITHUB,      /* <GW2>/addons/common/api.github.com              */
	DIR_NEXUS,                /* <GW2>/addons/Nexus                              */
	DIR_TEMP,                 /* <GW2>/addons/Nexus/Temp                         */
	DIR_FONTS,                /* <GW2>/addons/Nexus/Fonts                        */
	DIR_LOCALES,              /* <GW2>/addons/Nexus/Locales                      */
	DIR_STYLES,               /* <GW2>/addons/Nexus/Styles                       */
	DIR_TEXTURES,             /* <GW2>/addons/Nexus/Textures                     */

	DIR_DOCUMENTS,            /* <DOCUMENTS>                                     */
	DIR_DOCUMENTS_GW2,        /* <DOCUMENTS>/Guild Wars 2                        */
	DIR_GW2_INPUTBINDS,       /* <DOCUMENTS>/Guild Wars 2/InputBinds             */

	NexusDLL,                 /* <GW2>/d3d11.dll                                 */
	NexusDLL_Update,          /* <GW2>/d3d11.dll.update                          */
	NexusDLL_Old,             /* <GW2>/d3d11.dll.old                             */
	D3D11,                    /* <SYSTEM32>/d3d11.dll                            */
	D3D11Chainload,           /* <GW2>/d3d11_chainload.dll                       */

	Log,                      /* <GW2>/addons/Nexus/Nexus.log                    */
	LastCrashLog,             /* <GW2>/addons/Nexus/Crash.log                    */
	InputBinds,               /* <GW2>/addons/Nexus/InputBinds.json              */
	GameBinds,                /* <GW2>/addons/Nexus/GameBinds.xml                */
	Settings,                 /* <GW2>/addons/Nexus/Settings.json                */
	AddonConfigDefault,       /* <GW2>/addons/Nexus/AddonConfig.json             */
	ArcdpsIntegration,        /* <GW2>/addons/Nexus/arcdps_integration64.dll     */
	ThirdPartySoftwareReadme, /* <GW2>/addons/Nexus/THIRDPARTYSOFTWAREREADME.TXT */

	LocaleEN,                 /* <GW2>/addons/Nexus/Locales/en_Main.json         */
	LocaleDE,                 /* <GW2>/addons/Nexus/Locales/de_Main.json         */
	LocaleFR,                 /* <GW2>/addons/Nexus/Locales/fr_Main.json         */
	LocaleES,                 /* <GW2>/addons/Nexus/Locales/es_Main.json         */
	LocaleBR,                 /* <GW2>/addons/Nexus/Locales/br_Main.json         */
	LocaleCZ,                 /* <GW2>/addons/Nexus/Locales/cz_Main.json         */
	LocaleIT,                 /* <GW2>/addons/Nexus/Locales/it_Main.json         */
	LocalePL,                 /* <GW2>/addons/Nexus/Locales/pl_Main.json         */
	LocaleRU,                 /* <GW2>/addons/Nexus/Locales/ru_Main.json         */
	LocaleCN,                 /* <GW2>/addons/Nexus/Locales/cn_Main.json         */

	COUNT
};

#endif
