///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Config.h
/// Description  :  Configuration for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>

#include "CfgEnum.h"

///----------------------------------------------------------------------------------------------------
/// Config_t Struct
///----------------------------------------------------------------------------------------------------
struct Config_t
{
	bool        IsFavorite            = false;                  /* Marked as favorite.                                  */
	EUpdateMode UpdateMode            = EUpdateMode::Automatic; /* Behavior regarding updates.                          */
	bool        AllowPreReleases      = false;                  /* If the update provider supports pre-releases.        */
	bool        ShouldLoad            = true;                   /* Indicates if this addon should load next game start. */
	std::string DisableVersion        = "";                     /* Disable until update version comparison.             */
	uint32_t    LastGameBuild         = 0;                      /* The game build when the addon was last loaded.       */
	std::string LastName              = "";                     /* Last known name, for easier manual editing.          */
};

#endif
