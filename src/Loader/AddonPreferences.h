///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonPreferences.h
/// Description  :  Contains the definition for addon preferences.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDONPREFERENCES_H
#define ADDONPREFERENCES_H

#include <string>

#include "EUpdateMode.h"

///----------------------------------------------------------------------------------------------------
/// AddonPreferences Struct
///----------------------------------------------------------------------------------------------------
struct AddonPreferences
{
	EUpdateMode            UpdateMode;            /* Behavior regarding updates. */
	bool                   AllowPreReleases;      /* If the update provider supports pre-releases. */
	bool                   IsFavorite;            /* Marked as favorite. */
	bool                   IsDisabledUntilUpdate; /* Overrides EUpdateMode::Disable to prompt instead. */
};

#endif
