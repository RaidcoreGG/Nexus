#include "AddonDefinition.h"

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
		Load)
	{
		return true;
	}

	return false;
}

bool AddonDefinition::HasFlag(EAddonFlags aAddonFlag)
{
	return (bool)(Flags & aAddonFlag);
}