///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbBind.cpp
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IbBind.h"

namespace Raidcore::Engine::Input
{
	bool Bind_t::IsBound() const
	{
		return this->Device != EDevice::None && this->Code != 0;
	}

	bool operator==(const Bind_t& lhs, const Bind_t& rhs)
	{
		return lhs.Modifiers == rhs.Modifiers
			&& lhs.Device == rhs.Device
			&& lhs.Code == rhs.Code;
	}

	bool operator!=(const Bind_t& lhs, const Bind_t& rhs)
	{
		return !(lhs == rhs);
	}
}
