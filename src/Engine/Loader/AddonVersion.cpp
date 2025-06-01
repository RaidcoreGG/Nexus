///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonVersion_t.cpp
/// Description  :  AddonVersion_t struct definitions and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "AddonVersion.h"

#include <regex>

AddonVersion_t::AddonVersion_t(json& aJson)
{
	Major = !aJson["Major"].is_null()
		? aJson["Major"].get<signed short>()
		: 0;

	Minor = !aJson["Minor"].is_null()
		? aJson["Minor"].get<signed short>()
		: 0;

	Build = !aJson["Build"].is_null()
		? aJson["Build"].get<signed short>()
		: 0;

	Revision = !aJson["Revision"].is_null()
		? aJson["Revision"].get<signed short>()
		: 0;
}
AddonVersion_t::AddonVersion_t(std::string aVersionString)
{
	bool isSemVer = false;

	Major = 0;
	Minor = 0;
	Build = 0;
	Revision = 0;

	if (std::regex_match(aVersionString, std::regex(R"(v?\d+[.]\d+[.]\d+[.]\d+)")))
	{
		isSemVer = false; // is nexus versioning
	}
	else if (std::regex_match(aVersionString, std::regex(R"(v?\d+[.]\d+[.]\d+)")))
	{
		isSemVer = true; // is semver
	}
	else
	{
		return;
	}

	if (aVersionString._Starts_with("v"))
	{
		aVersionString = aVersionString.substr(1);
	}

	size_t pos = 0;
	int i = 0;
	while ((pos = aVersionString.find(".")) != std::string::npos)
	{
		switch (i)
		{
			case 0: Major = static_cast<unsigned short>(std::stoi(aVersionString.substr(0, pos))); break;
			case 1: Minor = static_cast<unsigned short>(std::stoi(aVersionString.substr(0, pos))); break;
			case 2: Build = static_cast<unsigned short>(std::stoi(aVersionString.substr(0, pos))); break;
			default: break;
		}
		i++;
		aVersionString.erase(0, pos + 1);
	}
	if (!isSemVer)
	{
		Revision = static_cast<unsigned short>(std::stoi(aVersionString));
	}
	else
	{
		/* this is left over, instead of the revision, so we assign it */
		Build = static_cast<unsigned short>(std::stoi(aVersionString));

		/* set revision to -1 to omit it */
		Revision = -1;
	}
}

std::string AddonVersion_t::string() const
{
	std::string str;
	str.append(std::to_string(Major) + ".");
	str.append(std::to_string(Minor) + ".");
	str.append(std::to_string(Build));

	if (Revision >= 0)
	{
		str.append("." + std::to_string(Revision));
	}
	else
	{
		// TODO: display (Alpha)/(Beta)/(rc) etc
	}

	return str;
}

bool operator>(AddonVersion_t lhs, AddonVersion_t rhs)
{
	if ((lhs.Major > rhs.Major) ||
		(lhs.Major == rhs.Major && lhs.Minor > rhs.Minor) ||
		(lhs.Major == rhs.Major && lhs.Minor == rhs.Minor && lhs.Build > rhs.Build) ||
		(lhs.Major == rhs.Major && lhs.Minor == rhs.Minor && lhs.Build == rhs.Build && lhs.Revision > rhs.Revision))
	{
		return true;
	}

	return false;
}
bool operator<(AddonVersion_t lhs, AddonVersion_t rhs)
{
	if ((lhs.Major < rhs.Major) ||
		(lhs.Major == rhs.Major && lhs.Minor < rhs.Minor) ||
		(lhs.Major == rhs.Major && lhs.Minor == rhs.Minor && lhs.Build < rhs.Build) ||
		(lhs.Major == rhs.Major && lhs.Minor == rhs.Minor && lhs.Build == rhs.Build && lhs.Revision < rhs.Revision))
	{
		return true;
	}

	return false;
}
bool operator==(AddonVersion_t lhs, AddonVersion_t rhs)
{
	if (lhs.Major == rhs.Major &&
		lhs.Minor == rhs.Minor &&
		lhs.Build == rhs.Build &&
		lhs.Revision == rhs.Revision)
	{
		return true;
	}

	return false;
}
bool operator!=(AddonVersion_t lhs, AddonVersion_t rhs)
{
	return !(lhs == rhs);
}
bool operator<=(AddonVersion_t lhs, AddonVersion_t rhs)
{
	return lhs < rhs || lhs == rhs;
}
bool operator>=(AddonVersion_t lhs, AddonVersion_t rhs)
{
	return lhs > rhs || lhs == rhs;
}