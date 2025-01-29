///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoaderConst.h
/// Description  :  Constants for the loader component.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADERCONST_H
#define LOADERCONST_H

#include "arcdps/ArcDPS.h"

/* Alias */
typedef ArcDPS::PluginInfo ArcdpsPlugin;

/* Log Channel */
constexpr const char* CH_LOADER                         = "Loader";

/* WndProc */
constexpr const UINT WM_ADDONDIRUPDATE                  = WM_USER + 101;

/* Delay */
constexpr int LOADER_WAITTIME_MS                        = 100;

/* File Extensions */
constexpr const char* EXT_DLL                           = ".dll";
constexpr const char* EXT_UPDATE                        = ".update";
constexpr const char* EXT_OLD                           = ".old";
constexpr const char* EXT_UNINSTALL                     = ".uninstall";

/* Preferences */
constexpr const char* PREFS_ADDON_SIGNATURE             = "Signature";
constexpr const char* PREFS_ADDON_ISLOADED              = "IsLoaded";
constexpr const char* PREFS_ADDON_NAME                  = "Name";
constexpr const char* PREFS_ADDON_UPDATEMODE            = "UpdateMode";
constexpr const char* PREFS_ADDON_ALLOWPRERELEASES      = "AllowPrereleases";
constexpr const char* PREFS_ADDON_ISFAVORITE            = "IsFavorite";
constexpr const char* PREFS_ADDON_ISDISABLEDUNTILUPDATE = "IsDisabledUntilUpdate";

#endif
