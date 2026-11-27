///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CfgEnum.h
/// Description  :  Enumerations for addon configs.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace Raidcore::GW2
{
	///----------------------------------------------------------------------------------------------------
	/// EUpdateMode Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EUpdateMode : uint32_t
	{
		None,
		Background, /* Automatically check for updates, but do nothing.        */
		Notify,     /* Automatically check for updates, but prompt to perform. */
		Automatic,  /* Automatically check and perform updates.                */
	};
}
