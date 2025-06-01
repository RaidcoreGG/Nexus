///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonVersion.h
/// Description  :  AddonVersion_t struct definitions and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDONVERSION_H
#define ADDONVERSION_H

#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// AddonVersion_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonVersion_t
{
	signed short	Major;
	signed short	Minor;
	signed short	Build;
	signed short	Revision;

	AddonVersion_t() = default;
	AddonVersion_t(signed short aMajor, signed short aMinor, signed short aBuild, signed short aRevision)
		: Major{ aMajor }
		, Minor{ aMinor }
		, Build{ aBuild }
		, Revision{ aRevision }
	{}
	AddonVersion_t(json& aJson);
	AddonVersion_t(std::string aVersionString);

	std::string string() const;
};

bool operator>(AddonVersion_t lhs, AddonVersion_t rhs);
bool operator<(AddonVersion_t lhs, AddonVersion_t rhs);
bool operator==(AddonVersion_t lhs, AddonVersion_t rhs);
bool operator!=(AddonVersion_t lhs, AddonVersion_t rhs);
bool operator<=(AddonVersion_t lhs, AddonVersion_t rhs);
bool operator>=(AddonVersion_t lhs, AddonVersion_t rhs);

#endif
