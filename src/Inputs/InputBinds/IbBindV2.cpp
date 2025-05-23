///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbBindV2.cpp
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IbBindV2.h"

#include "IbBind.h"

InputBind::InputBind(InputBindV1 aLegacyInputBind)
{
	this->Alt         = aLegacyInputBind.Alt;
	this->Ctrl        = aLegacyInputBind.Ctrl;
	this->Shift       = aLegacyInputBind.Shift;

	this->Device      = EInputDevice::Keyboard;
	this->Code        = aLegacyInputBind.Key;
}

bool InputBind::IsBound() const
{
	return this->Device != EInputDevice::None && this->Code != 0;
}

bool operator==(const InputBind& lhs, const InputBind& rhs)
{
	return lhs.Alt         == rhs.Alt &&
	       lhs.Ctrl        == rhs.Ctrl &&
	       lhs.Shift       == rhs.Shift &&
	       lhs.Device      == rhs.Device &&
	       lhs.Code        == rhs.Code;
}

bool operator!=(const InputBind& lhs, const InputBind& rhs)
{
	return !(lhs == rhs);
}
