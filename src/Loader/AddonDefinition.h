#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "API/AddonAPI.h"
#include "EAddonFlags.h"
#include "EUpdateProvider.h"
#include "AddonVersion.h"

typedef void (*ADDON_LOAD)(AddonAPI_t* aAPI);
typedef void (*ADDON_UNLOAD)();

struct AddonDef_t
{
	/* required */
	signed int      Signature;      /* Raidcore Addon_t ID, set to random unqiue negative integer if not on Raidcore */
	signed int      APIVersion;     /* Determines which AddonAPI_t struct revision the Loader will pass, use the NEXUS_API_VERSION define from Nexus.h */
	const char*     Name;           /* Name of the addon as shown in the library */
	AddonVersion_t    Version;
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

	static void Copy(AddonDef_t* aSrc, AddonDef_t** aDst);
	static void Free(AddonDef_t** aDefinitions);
};

#endif