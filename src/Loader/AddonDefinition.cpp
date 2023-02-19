#include "AddonDefinition.h"

bool AddonDefinition::HasMinimumRequirements()
{
	if (Signature != 0 &&
		Name &&
		Version &&
		Author &&
		Description &&
		Load &&
		Unload)
	{
		return true;
	}

	return false;
}