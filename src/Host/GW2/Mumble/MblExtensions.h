///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblExtensions.h
/// Description  :  Extensions for Mumble types.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <cmath>

#include "thirdparty/mumble/Mumble.h"

///----------------------------------------------------------------------------------------------------
/// Mumble Namespace
///----------------------------------------------------------------------------------------------------
namespace Mumble
{
	inline bool operator==(const Identity& lhs, const Identity& rhs)
	{
		return strcmp(lhs.Name, rhs.Name) == 0
			&& lhs.Profession     == rhs.Profession
			&& lhs.Specialization == rhs.Specialization
			&& lhs.Race           == rhs.Race
			&& lhs.MapID          == rhs.MapID
			&& lhs.WorldID        == rhs.WorldID
			&& lhs.TeamColorID    == rhs.TeamColorID
			&& lhs.IsCommander    == rhs.IsCommander
			&& lhs.FOV            == rhs.FOV 
			&& lhs.UISize         == rhs.UISize;
	}

	inline bool operator!=(const Identity& lhs, const Identity& rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator==(const Vector2& lhs, const Vector2& rhs)
	{
		return trunc(1000. * lhs.X) == trunc(1000. * rhs.X)
			&& trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y);
	}

	inline bool operator!=(const Vector2& lhs, const Vector2& rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator==(const Vector3& lhs, const Vector3& rhs)
	{
		return trunc(1000. * lhs.X) == trunc(1000. * rhs.X)
			&& trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y)
			&& trunc(1000. * lhs.Z) == trunc(1000. * rhs.Z);
	}

	inline bool operator!=(const Vector3& lhs, const Vector3& rhs)
	{
		return !(lhs == rhs);
	}
};
