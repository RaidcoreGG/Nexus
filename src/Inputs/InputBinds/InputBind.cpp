///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  InputBind.cpp
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "InputBind.h"

InputBind::InputBind(LegacyInputBind aLegacyInputBind)
{
	Alt = aLegacyInputBind.Alt;
	Ctrl = aLegacyInputBind.Ctrl;
	Shift = aLegacyInputBind.Shift;

	Type = EInputBindType::Keyboard;
	Code = aLegacyInputBind.Key;
}

bool InputBind::IsBound()
{
	return Type != EInputBindType::None && Code != 0;
}

bool operator==(const InputBind& lhs, const InputBind& rhs)
{
	return	lhs.Alt		== rhs.Alt &&
			lhs.Ctrl	== rhs.Ctrl &&
			lhs.Shift	== rhs.Shift &&
			lhs.Type	== rhs.Type &&
			lhs.Code	== rhs.Code;
}

bool operator!=(const InputBind& lhs, const InputBind& rhs)
{
	return	!(lhs == rhs);
}
