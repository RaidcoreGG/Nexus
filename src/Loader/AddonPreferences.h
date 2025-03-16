///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonPreferences.h
/// Description  :  Contains the definition for addon preferences.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDONPREFERENCES_H
#define ADDONPREFERENCES_H

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "EUpdateMode.h"
#include "Addon.h"

///----------------------------------------------------------------------------------------------------
/// AddonPrefs Struct
///----------------------------------------------------------------------------------------------------
struct AddonPrefs
{
	EUpdateMode UpdateMode;            /* Behavior regarding updates.                          */
	bool        AllowPreReleases;      /* If the update provider supports pre-releases.        */
	bool        IsFavorite;            /* Marked as favorite.                                  */
	bool        IsDisabledUntilUpdate; /* Overrides EUpdateMode::Disable to prompt instead.    */
	bool        ShouldLoad;            /* Indicates if this addon should load next game start. */
	int         LastGameBuild;         /* The game build when the addon was last loaded.       */

	CAddon*     Owner;

	AddonPrefs() = default;
	AddonPrefs(json& aJson);

	json ToJSON();
};

#endif
