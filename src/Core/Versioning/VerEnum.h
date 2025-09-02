///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  VerEnum.h
/// Description  :  Enumerations for versioning.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef VERENUM_H
#define VERENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EVersionFormat Enumeration
///----------------------------------------------------------------------------------------------------
enum class EVersionFormat : uint32_t
{
	None,

	U64_4XS16,
	U48_3XS16
};

#endif
