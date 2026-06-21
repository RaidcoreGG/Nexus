///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LdrAddonBase.cpp
/// Description  :  Contains the interface definition for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LdrAddonBase.h"

std::filesystem::path IAddon::GetLocation() const
{
	return this->Location;
}

IAddon* IAddon::GetBase()
{
	return dynamic_cast<IAddon*>(this);
}

MD5_t IAddon::GetMD5() const
{
	return this->MD5;
}

bool IAddon::IsLoaded() const
{
	switch (this->State)
	{
		default:
		case EAddonState::None:
		case EAddonState::NotLoaded:
		{
			return false;
		}
		case EAddonState::Loaded:
		{
			return true;
		}
	}
}

bool IAddon::OwnsAddress(void* aAddress) const
{
	if (this->Module && this->ModuleSize > 0)
	{
		void* startAddress = this->Module;
		void* endAddress = ((PBYTE)this->Module) + this->ModuleSize;

		if (aAddress >= startAddress && aAddress <= endAddress)
		{
			return true;
		}
	}

	return false;
}

void IAddon::SetLocation(std::filesystem::path aLocation)
{
	this->Location = aLocation;
}
