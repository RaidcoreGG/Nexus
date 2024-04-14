#include "AddonDefinition.h"

std::string AddonVersion::ToString()
{
	std::string str;
	str.append(std::to_string(Major) + ".");
	str.append(std::to_string(Minor) + ".");
	str.append(std::to_string(Build) + ".");
	str.append(std::to_string(Revision));
	return str;
}

AddonVersion VersionFromJson(json aJson)
{
	AddonVersion version{};

	if (!aJson["Major"].is_null())
	{
		aJson["Major"].get_to(version.Major);
	}
	if (!aJson["Minor"].is_null())
	{
		aJson["Minor"].get_to(version.Minor);
	}
	if (!aJson["Build"].is_null())
	{
		aJson["Build"].get_to(version.Build);
	}
	if (!aJson["Revision"].is_null())
	{
		aJson["Revision"].get_to(version.Revision);
	}

	return version;
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