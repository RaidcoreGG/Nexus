///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  StyleEnum.h
/// Description  :  Enumerations for the UI styles.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GUI Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GUI
{
	///----------------------------------------------------------------------------------------------------
	/// EUIStyle Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EUIStyle : uint32_t
	{
		User,
		Nexus,
		ImGui_Classic,
		ImGui_Light,
		ImGui_Dark,
		ArcDPS_Default,
		ArcDPS_Current, /* If available. */
		File,
		Code
	};
}
