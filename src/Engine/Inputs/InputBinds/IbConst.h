///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbConst.h
/// Description  :  Constant data for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "IbBind.h"

namespace Raidcore::Engine::Input
{
	///----------------------------------------------------------------------------------------------------
	/// IBFromString:
	/// 	Helper function to create a Bind_t from a string.
	///----------------------------------------------------------------------------------------------------
	Bind_t IBFromString(std::string aInputBind);

	///----------------------------------------------------------------------------------------------------
	/// IBToString:
	/// 	Helper function to get the display string of a Bind_t.
	///----------------------------------------------------------------------------------------------------
	std::string IBToString(const Bind_t& aKebyind, bool aPadded = false);
}
