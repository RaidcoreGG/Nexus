///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibraryAddon.cpp
/// Description  :  Contains the definition for a library addon listing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LibraryAddon.h"

#include "Consts.h"

constexpr const char* K_SIGNATURE   = "id";
constexpr const char* K_NAME        = "name";
constexpr const char* K_AUTHOR      = "author";
constexpr const char* K_DESCRIPTION = "description";
constexpr const char* K_DOWNLOADURL = "download";
constexpr const char* K_POLICY      = "addon_policy_tier";
constexpr const char* K_FILENAME    = "filename";

LibraryAddon::LibraryAddon(json& aJson)
{
	this->Signature        = 0;
	this->Name             = NULLSTR;
	this->Author           = NULLSTR;
	this->Description      = NULLSTR;
	this->DownloadURL      = NULLSTR;
	this->PolicyTier       = 0;
	this->FriendlyFilename = NULLSTR;

	if (!aJson[K_SIGNATURE].is_null())
	{
		aJson[K_SIGNATURE].get_to(this->Signature);
	}

	if (!aJson[K_NAME].is_null())
	{
		aJson[K_NAME].get_to(this->Name);
	}

	if (!aJson[K_AUTHOR].is_null())
	{
		aJson[K_AUTHOR].get_to(this->Author);
	}

	if (!aJson[K_DESCRIPTION].is_null())
	{
		aJson[K_DESCRIPTION].get_to(this->Description);
	}

	if (!aJson[K_DOWNLOADURL].is_null())
	{
		aJson[K_DOWNLOADURL].get_to(this->DownloadURL);
	}

	if (!aJson[K_POLICY].is_null())
	{
		aJson[K_POLICY].get_to(this->PolicyTier);
	}

	if (!aJson[K_FILENAME].is_null())
	{
		aJson[K_FILENAME].get_to(this->FriendlyFilename);
	}
}
