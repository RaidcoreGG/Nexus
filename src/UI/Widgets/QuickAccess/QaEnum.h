///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaEnum.h
/// Description  :  Enumerations for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QAENUM_H
#define QAENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EQaPosition Enumeration
///----------------------------------------------------------------------------------------------------
enum class EQaPosition : uint32_t
{
	Extend,
	Under,
	Bottom,
	Custom
};

///----------------------------------------------------------------------------------------------------
/// EQaVisibility Enumeration
///----------------------------------------------------------------------------------------------------
enum class EQaVisibility : uint32_t
{
	AlwaysShow,
	Gameplay,
	OutOfCombat,
	InCombat,
	Hide
};

#endif
