#ifndef ADDONDEF_H
#define ADDONDEF_H

#include <string>

#include "AddonAPI.h"
#include "EAddonFlags.h"
#include "EUpdateProvider.h"

typedef void (*ADDON_LOAD)(AddonAPI* aAPI);
typedef void (*ADDON_UNLOAD)();

struct AddonVersion
{
	signed short	Major;
	signed short	Minor;
	signed short	Build;
	signed short	Revision;

	std::string ToString()
	{
		std::string str;
		str.append(std::to_string(Major) + ".");
		str.append(std::to_string(Minor) + ".");
		str.append(std::to_string(Build) + ".");
		str.append(std::to_string(Revision));
		return str;
	}
};

bool operator>(AddonVersion lhs, AddonVersion rhs);
bool operator<(AddonVersion lhs, AddonVersion rhs);

struct AddonDefinition
{
	/* required */
	signed int      Signature;      /* Raidcore Addon ID, set to random unqiue negative integer if not on Raidcore */
	signed int		APIVersion;		/* Determines which AddonAPI struct revision the Loader will pass, use the NEXUS_API_VERSION define from Nexus.h */
	const char*     Name;           /* Name of the addon as shown in the library */
	AddonVersion	Version;
	const char*     Author;         /* Author of the addon */
	const char*     Description;    /* Short description */
	ADDON_LOAD      Load;           /* Pointer to Load Function of the addon */
	ADDON_UNLOAD    Unload;         /* Pointer to Unload Function of the addon. Not required if EAddonFlags::DisableHotloading is set. */
	EAddonFlags     Flags;          /* Information about the addon */

	/* update fallback */
	EUpdateProvider Provider;       /* What platform is the the addon hosted on */
	const char*     UpdateLink;     /* Link to the update resource */

	/* internal */
	bool HasMinimumRequirements();
	bool HasFlag(EAddonFlags aAddonFlag);
};

#endif