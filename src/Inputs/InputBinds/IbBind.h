///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbBind.h
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbEnum.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::Input Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::Input
{
	///----------------------------------------------------------------------------------------------------
	/// InputBindV1_t Struct
	/// 	Old Keybind struct used for backwards compatibility within APIs.
	///----------------------------------------------------------------------------------------------------
	struct InputBindV1_t
	{
		unsigned short Key;
		bool           Alt;
		bool           Ctrl;
		bool           Shift;
	};
}
