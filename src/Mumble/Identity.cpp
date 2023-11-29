#include "Identity.h"

namespace Mumble
{
	bool operator==(const Identity& lhs, const Identity& rhs)
	{
		if (strcmp(lhs.Name, rhs.Name) == 0 &&
			lhs.Profession == rhs.Profession &&
			lhs.Specialization == rhs.Specialization &&
			lhs.Race == rhs.Race &&
			lhs.MapID == rhs.MapID &&
			lhs.WorldID == rhs.WorldID &&
			lhs.TeamColorID == rhs.TeamColorID &&
			lhs.IsCommander == rhs.IsCommander &&
			lhs.FOV == rhs.FOV &&
			lhs.UISize == rhs.UISize)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Identity& lhs, const Identity& rhs)
	{
		return !(lhs == rhs);
	}
}