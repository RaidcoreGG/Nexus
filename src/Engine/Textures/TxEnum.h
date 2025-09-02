///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TxEnum.h
/// Description  :  Enumerations for textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef TXENUM_H
#define TXENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// ETextureStage Enumeration
///----------------------------------------------------------------------------------------------------
enum class ETextureStage : uint32_t
{
	None        = 0,
	Prepare     = 1,
	Ready       = 2,
	Done        = 3,
	INVALID     = UINT32_MAX
};

#endif
