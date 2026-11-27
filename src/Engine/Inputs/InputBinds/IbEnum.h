///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbEnum.h
/// Description  :  InputBind enums.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <windows.h>

namespace Engine::Input
{
	///----------------------------------------------------------------------------------------------------
	/// EDevice Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EDevice : uint16_t
	{
		None,
		Keyboard,
		Mouse
	};

	///----------------------------------------------------------------------------------------------------
	/// EMouseButtons Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EMouseButtons : uint16_t
	{
		None,
		LMB,
		RMB,
		MMB,
		M4,
		M5
	};

	///----------------------------------------------------------------------------------------------------
	/// EModifiers Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EModifiers : uint16_t
	{
		None  = 0,
		Shift = 1 << 0,
		Ctrl  = 1 << 1,
		Alt   = 1 << 2
	};

#ifdef DEFINE_ENUM_FLAG_OPERATORS
	DEFINE_ENUM_FLAG_OPERATORS(EModifiers);
#endif

	///----------------------------------------------------------------------------------------------------
	/// EIbHandlerType Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EIbHandlerType : uint32_t
	{
		None,
		DownAsync,        /* Type 1 */
		DownReleaseAsync, /* Type 2 */
		DownRelease       /* Type 3 */
	};
}
