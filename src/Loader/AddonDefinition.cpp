#include "AddonDefinition.h"

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