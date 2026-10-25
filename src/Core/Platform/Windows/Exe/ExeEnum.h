///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExeEnum.h
/// Description  :  Enumerations for executable files.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// Raidcore::Core::Platform::Windows Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Core::Platform::Windows
{
	///----------------------------------------------------------------------------------------------------
	/// EArchitecture Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EArchitecture : uint32_t
	{
		None,    /* Not a readable executable. */
		Unknown,
		x86,
		x64,
		ARM64
	};

#if defined(_M_X64)
	constexpr EArchitecture ARCH_CURRENT = EArchitecture::x64;
#elif defined(_M_IX86)
	constexpr EArchitecture ARCH_CURRENT = EArchitecture::x86;
#elif defined(_M_ARM64)
	constexpr EArchitecture ARCH_CURRENT = EArchitecture::ARM64;
#else
	constexpr EArchitecture ARCH_CURRENT = EArchitecture::Unknown;
#endif
}
