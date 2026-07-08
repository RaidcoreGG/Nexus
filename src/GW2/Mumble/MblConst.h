///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblConst.h
/// Description  :  Constant data for MumbleLink.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "thirdparty/mumble/Mumble.h"

constexpr const float SC_SMALL  = 0.90f;
constexpr const float SC_NORMAL = 1.00f;
constexpr const float SC_LARGE  = 1.11f;
constexpr const float SC_LARGER = 1.22f;

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2 Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2
{
	///----------------------------------------------------------------------------------------------------
	/// GetScalingFactor:
	/// 	Returns the scaling factor for the given the UISize enum.
	///----------------------------------------------------------------------------------------------------
	float GetScalingFactor(Mumble::EUIScale aSize);
};
