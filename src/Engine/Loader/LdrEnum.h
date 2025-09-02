///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LdrEnum.h
/// Description  :  Enumerations for the loader.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LDRENUM_H
#define LDRENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EAddonState Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonState : uint32_t
{
	None,
	NotLoaded,
	Loaded
};

#endif
