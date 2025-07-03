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

void IAddon::SetLocation(std::filesystem::path aLocation)
{
	this->Location = aLocation;
}

IAddon* IAddon::GetBase()
{
	return dynamic_cast<IAddon*>(this);
}

MD5_t IAddon::GetMD5() const
{
	return this->MD5;
}

bool IAddon::OwnsAddress(void* aAddress)
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
