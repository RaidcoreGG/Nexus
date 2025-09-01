///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonUpdater.cpp
/// Description  :  Addon provider implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

bool CAddon::SupportsPreReleases() const
{
	if (this->NexusAddonDefV1)
	{
		switch (this->NexusAddonDefV1->Provider)
		{
			case EUpdateProvider::GitHub:
			{
				return true;
			}
		}
	}

	return false;
}

std::string CAddon::GetProjectPageURL() const
{
	if (this->NexusAddonDefV1)
	{
		switch (this->NexusAddonDefV1->Provider)
		{
			case EUpdateProvider::GitHub:
			{
				return this->NexusAddonDefV1->UpdateLink;
			}
		}
	}

	return "";
}

void CAddon::CheckForUpdateInternal()
{
	if (!this->IsUpdateAvailable())
	{
		return;
	}

	return;
}

bool CAddon::UpdateInternal()
{
	return false;
}

bool CAddon::UpdateViaRaidcore()
{
	return false;
}

bool CAddon::UpdateViaGitHub()
{
	return false;
}

bool CAddon::UpdateViaDirect()
{
	return false;
}
