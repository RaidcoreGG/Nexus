#include "AddonDefinition.h"

bool AddonDef_t::HasMinimumRequirements()
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

bool AddonDef_t::HasFlag(EAddonFlags aAddonFlag)
{
	return (bool)(Flags & aAddonFlag);
}

void AddonDef_t::Copy(AddonDef_t* aSrc, AddonDef_t** aDst)
{
	if (aSrc == nullptr)
	{
		*aDst = new AddonDef_t{};
		return;
	}

	// Allocate new memory and copy data, copy strings
	*aDst = new AddonDef_t(*aSrc);
	(*aDst)->Name = _strdup(aSrc->Name);
	(*aDst)->Author = _strdup(aSrc->Author);
	(*aDst)->Description = _strdup(aSrc->Description);
	(*aDst)->UpdateLink = aSrc->UpdateLink ? _strdup(aSrc->UpdateLink) : nullptr;
}

void AddonDef_t::Free(AddonDef_t** aDefinitions)
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