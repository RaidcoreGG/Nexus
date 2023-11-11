#include "AddonDefinition.h"

bool operator>(AddonVersion lhs, AddonVersion rhs)
{
	unsigned long long lLong;
	unsigned long long rLong;

	std::stringstream oss;
	oss << std::left << std::setfill('0') << std::setw(4) << lhs.Major;
	oss << std::left << std::setfill('0') << std::setw(2) << lhs.Minor;
	oss << std::left << std::setfill('0') << std::setw(2) << lhs.Build;
	oss << std::left << std::setfill('0') << std::setw(4) << lhs.Revision;
	std::string lStr = oss.str();

	oss.str("");
	oss << std::left << std::setfill('0') << std::setw(4) << rhs.Major;
	oss << std::left << std::setfill('0') << std::setw(2) << rhs.Minor;
	oss << std::left << std::setfill('0') << std::setw(2) << rhs.Build;
	oss << std::left << std::setfill('0') << std::setw(4) << rhs.Revision;
	std::string rStr = oss.str();

	lLong = std::stoull(lStr);
	rLong = std::stoull(rStr);

	return lLong > rLong;
}
bool operator<(AddonVersion lhs, AddonVersion rhs)
{
	return !(lhs > rhs);
}

bool AddonDefinition::HasMinimumRequirements()
{
	if (Signature != 0 &&
		Name &&
		Author &&
		Description &&
		Load &&
		Unload)
	{
		return true;
	}

	return false;
}

bool AddonDefinition::HasFlag(EAddonFlags aAddonFlag)
{
	return (bool)(Flags & aAddonFlag);
}