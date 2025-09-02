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

void CAddon::CheckUpdateInternal()
{
	if (this->IsUpdateAvailable())
	{
		/* Already checked. */
		return;
	}

	return;
}

bool CAddon::CheckUpdateViaGitHub()
{
	if (this->IsUpdateAvailable())
	{
		/* Already checked. */
		return true;
	}

	return false;
}

bool CAddon::CheckUpdateViaDirect()
{
	if (this->IsUpdateAvailable())
	{
		/* Already checked. */
		return true;
	}

	return false;
}

bool CAddon::UpdateInternal()
{
	this->Logger->Trace(CH_ADDON, "CAddon::UpdateInternal(%s)", this->Location.string().c_str());

	if (!this->IsUpdateAvailable())
	{
		this->Logger->Debug(CH_ADDON, "Can't update. No update available. (%s)", this->Location.string().c_str());
		return false;
	}

	return false;
}
