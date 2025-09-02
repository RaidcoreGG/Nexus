///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonDetails.cpp
/// Description  :  Addon details implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

std::string CAddon::GetName()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Name;
	}

	if (this->ArcExtensionDef)
	{
		return this->ArcExtensionDef->Name;
	}

	if (!this->Location.empty())
	{
		return this->Location.filename().string();
	}

	return "";
}

std::string CAddon::GetAuthor()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Author;
	}

	return "";
}

std::string CAddon::GetDescription()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Description;
	}

	return "";
}

std::string CAddon::GetVersion()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Version.string();
	}

	if (this->ArcExtensionDef)
	{
		return this->ArcExtensionDef->Build;
	}

	return "";
}
