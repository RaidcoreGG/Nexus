///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonPreferences.cpp
/// Description  :  Contains the definition for addon preferences.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "AddonPreferences.h"

#include "Consts.h"

/* Migration Keys */
constexpr const char* KM_PAUSEUPDATES      = "IsPausingUpdates";
constexpr const char* KM_ISLOADED          = "IsLoaded";

/* Active Keys */
constexpr const char* K_NAME               = "Name";
constexpr const char* K_UPDATEMODE         = "UpdateMode";
constexpr const char* K_PRERELEASES        = "AllowPrereleases";
constexpr const char* K_ISFAVORITE         = "IsFavorite";
constexpr const char* K_DISABLEUNTILUPDATE = "IsDisabledUntilUpdate";
constexpr const char* K_SHOULDLOAD         = "ShouldLoad";
constexpr const char* K_LASTGAMEBUILD      = "LastGameBuild";

AddonPrefs::AddonPrefs(json& aJson)
{
	this->UpdateMode            = EUpdateMode::AutoUpdate;
	this->AllowPreReleases      = false;
	this->IsFavorite            = false;
	this->IsDisabledUntilUpdate = false;
	this->ShouldLoad            = true;
	this->LastGameBuild         = 0;

	/* Migrate legacy update setting. */
	if (!aJson[KM_PAUSEUPDATES].is_null())
	{
		bool pauseUpdates = false;
		aJson[KM_PAUSEUPDATES].get_to(pauseUpdates);

		this->UpdateMode = pauseUpdates == true ? EUpdateMode::Background : EUpdateMode::AutoUpdate;
	}

	/* Migrate legacy load state. */
	if (!aJson[KM_ISLOADED].is_null())
	{
		aJson[KM_ISLOADED].get_to(this->ShouldLoad);
	}

	/* Get preferences. */

	if (!aJson[K_UPDATEMODE].is_null())
	{
		aJson[K_UPDATEMODE].get_to(this->UpdateMode);
	}

	if (!aJson[K_PRERELEASES].is_null())
	{
		aJson[K_PRERELEASES].get_to(this->AllowPreReleases);
	}

	if (!aJson[K_ISFAVORITE].is_null())
	{
		aJson[K_ISFAVORITE].get_to(this->IsFavorite);
	}

	if (!aJson[K_DISABLEUNTILUPDATE].is_null())
	{
		aJson[K_DISABLEUNTILUPDATE].get_to(this->IsDisabledUntilUpdate);
	}

	if (!aJson[K_SHOULDLOAD].is_null())
	{
		aJson[K_SHOULDLOAD].get_to(this->ShouldLoad);
	}

	if (!aJson[K_LASTGAMEBUILD].is_null())
	{
		aJson[K_LASTGAMEBUILD].get_to(this->LastGameBuild);
	}
}

json AddonPrefs::ToJSON()
{
	/* Name and Signature have to be added by the PrefMgr */
	return json {
		{ K_NAME,               this->Owner ? this->Owner->GetName() : NULLSTR },
		{ K_UPDATEMODE,         this->UpdateMode                               },
		{ K_PRERELEASES,        this->AllowPreReleases                         },
		{ K_ISFAVORITE,         this->IsFavorite                               },
		{ K_DISABLEUNTILUPDATE, this->IsDisabledUntilUpdate                    },
		{ K_SHOULDLOAD,         this->ShouldLoad                               },
		{ K_LASTGAMEBUILD,      this->LastGameBuild                            }
	};
}
