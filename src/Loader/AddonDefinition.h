#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "AddonAPI.h"
#include "EAddonFlags.h"
#include "EUpdateProvider.h"
#include "AddonVersion.h"

typedef void (*ADDON_LOAD)(AddonAPI* aAPI);
typedef void (*ADDON_UNLOAD)();

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

	static void Copy(AddonDefinition* aSrc, AddonDefinition** aDst);
	static void Free(AddonDefinition** aDefinitions);
};

#endif