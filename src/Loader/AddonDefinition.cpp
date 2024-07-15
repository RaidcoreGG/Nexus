#include "AddonDefinition.h"

#include <regex>

AddonVersion::AddonVersion(json& aJson)
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

	Revision =	!aJson["Revision"].is_null()
				? aJson["Revision"].get<signed short>()
				: 0;
}

AddonVersion::AddonVersion(std::string aVersionString)
{
	bool isSemVer = false;

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

std::string AddonVersion::string()
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

bool operator>(AddonVersion lhs, AddonVersion rhs)
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
bool operator<(AddonVersion lhs, AddonVersion rhs)
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
bool operator==(AddonVersion lhs, AddonVersion rhs)
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
bool operator!=(AddonVersion lhs, AddonVersion rhs)
{
	return !(lhs == rhs);
}
bool operator<=(AddonVersion lhs, AddonVersion rhs)
{
	return lhs < rhs || lhs == rhs;
}
bool operator>=(AddonVersion lhs, AddonVersion rhs)
{
	return lhs > rhs || lhs == rhs;
}

bool AddonDefinition::HasMinimumRequirements()
{
	if (Signature != 0 &&
		Name &&
		Author &&
		Description &&
		Load &&
		(HasFlag(EAddonFlags::DisableHotloading) || Unload))
	{
		return true;
	}

	return false;
}

bool AddonDefinition::HasFlag(EAddonFlags aAddonFlag)
{
	return (bool)(Flags & aAddonFlag);
}

void AddonDefinition::Copy(AddonDefinition* aSrc, AddonDefinition** aDst)
{
	if (aSrc == nullptr)
	{
		*aDst = new AddonDefinition{};
		return;
	}

	// Allocate new memory and copy data, copy strings
	*aDst = new AddonDefinition(*aSrc);
	(*aDst)->Name = _strdup(aSrc->Name);
	(*aDst)->Author = _strdup(aSrc->Author);
	(*aDst)->Description = _strdup(aSrc->Description);
	(*aDst)->UpdateLink = aSrc->UpdateLink ? _strdup(aSrc->UpdateLink) : nullptr;
}

void AddonDefinition::Free(AddonDefinition** aDefinitions)
{
	if (*aDefinitions == nullptr) { return; }

	free((char*)(*aDefinitions)->Name);
	free((char*)(*aDefinitions)->Author);
	free((char*)(*aDefinitions)->Description);
	if ((*aDefinitions)->UpdateLink)
	{
		free((char*)(*aDefinitions)->UpdateLink);
	}
	delete* aDefinitions;

	*aDefinitions = nullptr;
}