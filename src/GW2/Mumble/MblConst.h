///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblConst.h
/// Description  :  Constant data for MumbleLink.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MBLCONST_H
#define MBLCONST_H

#include "thirdparty/mumble/Mumble.h"

constexpr const float SC_SMALL  = 0.90f;
constexpr const float SC_NORMAL = 1.00f;
constexpr const float SC_LARGE  = 1.11f;
constexpr const float SC_LARGER = 1.22f;

///----------------------------------------------------------------------------------------------------
/// Mumble Namespace
///----------------------------------------------------------------------------------------------------
namespace Mumble
{
	///----------------------------------------------------------------------------------------------------
	/// GetScalingFactor:
	/// 	Returns the scaling factor for the given the UISize enum.
	///----------------------------------------------------------------------------------------------------
	float GetScalingFactor(EUIScale aSize);
};

#endif
