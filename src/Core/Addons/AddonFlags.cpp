///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonFlags.cpp
/// Description  :  Addon flags implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

bool CAddon::IsRunningAction() const
{
	return (this->Flags & EAddonFlags::RunningAction) == EAddonFlags::RunningAction;
}

bool CAddon::IsDestroying() const
{
	return (this->Flags & EAddonFlags::Destroying) == EAddonFlags::Destroying;
}

bool CAddon::IsFileLocked() const
{
	return (this->Flags & EAddonFlags::FileLocked) == EAddonFlags::FileLocked;
}

bool CAddon::IsStateLocked() const
{
	return (this->Flags & EAddonFlags::StateLocked) == EAddonFlags::StateLocked;
}

bool CAddon::IsMissingRequirements() const
{
	return (this->Flags & EAddonFlags::MissingReqs) == EAddonFlags::MissingReqs;
}

bool CAddon::IsUninstalled() const
{
	return (this->Flags & EAddonFlags::Uninstalled) == EAddonFlags::Uninstalled;
}

bool CAddon::IsUpdateAvailable() const
{
	return (this->Flags & EAddonFlags::UpdateAvailable) == EAddonFlags::UpdateAvailable;
}
