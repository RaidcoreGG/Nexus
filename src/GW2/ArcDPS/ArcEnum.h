///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcEnum.h
/// Description  :  Enumerations for ArcDPS API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2::ArcDPS Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2::ArcDPS
{
	///----------------------------------------------------------------------------------------------------
	/// EAddExtResult Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EAddExtResult : int32_t
	{
		NotInitialized = -1,
		Success = 0,
		ExtensionError,
		ImGuiVersionMismatch,
		Obsolete,
		Duplicate,
		MissingExports,
		MissingInit,
		LoadLibraryFailed
	};
}
