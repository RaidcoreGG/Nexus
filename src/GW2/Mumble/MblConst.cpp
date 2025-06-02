///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblConst.cpp
/// Description  :  Constant data for MumbleLink.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "MblConst.h"

namespace Mumble
{
	float GetScalingFactor(EUIScale aSize)
	{
		switch (aSize)
		{
			case EUIScale::Small:  { return SC_SMALL;  }
			default:
			case EUIScale::Normal: { return SC_NORMAL; }
			case EUIScale::Large:  { return SC_LARGE;  }
			case EUIScale::Larger: { return SC_LARGER; }
		}
	}
};
