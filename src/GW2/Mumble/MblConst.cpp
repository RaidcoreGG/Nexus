///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblConst.cpp
/// Description  :  Constant data for MumbleLink.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "MblConst.h"

namespace Raidcore::Nexus::GW2
{
	float GetScalingFactor(Mumble::EUIScale aSize)
	{
		switch (aSize)
		{
			case Mumble::EUIScale::Small:  { return SC_SMALL;  }
			default:
			case Mumble::EUIScale::Normal: { return SC_NORMAL; }
			case Mumble::EUIScale::Large:  { return SC_LARGE;  }
			case Mumble::EUIScale::Larger: { return SC_LARGER; }
		}
	}
};
