///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcEnum.h
/// Description  :  Enumerations for ArcDPS API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ARCENUM_H
#define ARCENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// ArcDPS Namespace
///----------------------------------------------------------------------------------------------------
namespace ArcDPS
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

#endif
