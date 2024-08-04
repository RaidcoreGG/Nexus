///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonVersion.h
/// Description  :  AddonVersion struct definitions and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDONVERSION_H
#define ADDONVERSION_H

#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// AddonVersion Struct
///----------------------------------------------------------------------------------------------------
struct AddonVersion
{
	signed short	Major;
	signed short	Minor;
	signed short	Build;
	signed short	Revision;

	AddonVersion() = default;
	AddonVersion(signed short aMajor, signed short aMinor, signed short aBuild, signed short aRevision)
		: Major{ aMajor }
		, Minor{ aMinor }
		, Build{ aBuild }
		, Revision{ aRevision }
	{}
	AddonVersion(json& aJson);
	AddonVersion(std::string aVersionString);

	std::string string();
};

bool operator>(AddonVersion lhs, AddonVersion rhs);
bool operator<(AddonVersion lhs, AddonVersion rhs);
bool operator==(AddonVersion lhs, AddonVersion rhs);
bool operator!=(AddonVersion lhs, AddonVersion rhs);
bool operator<=(AddonVersion lhs, AddonVersion rhs);
bool operator>=(AddonVersion lhs, AddonVersion rhs);

#endif
