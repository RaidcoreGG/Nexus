///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonDefinition.h
/// Description  :  Contains the definition for aaddons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "API/ApiDefs.h"
#include "AddonVersion.h"
#include "EAddonFlags.h"
#include "EUpdateProvider.h"

typedef void (*ADDON_LOAD)(AddonAPI* aAPI);
typedef void (*ADDON_UNLOAD)();

///----------------------------------------------------------------------------------------------------
/// AddonDefinition Struct
///----------------------------------------------------------------------------------------------------
struct AddonDefinition
{
	signed int      Signature;   /* [Required] Unique addon ID. Set to random unqiue negative integer if not hosted on Raidcore. */
	signed int      APIVersion;  /* [Required] Determines the API revision that's passed to the Load function. Use the "NEXUS_API_VERSION" define from Nexus.h */
	const char*     Name;        /* [Required] */
	AddonVersion    Version;     /* [Required] */
	const char*     Author;      /* [Required] */
	const char*     Description; /* [Required] */
	ADDON_LOAD      Load;        /* [Required] Pointer to Load Function. */
	ADDON_UNLOAD    Unload;      /* [Optional] Pointer to Unload Function. Not setting it implicitly declares EAddonFlags::DisableHotloading. */
	EAddonFlags     Flags;       /* [Optional] Additional flags changing some behaviors regarding the addon. */

	EUpdateProvider Provider;    /* [Optional] */
	const char*     UpdateLink;  /* [Optional] URL to update resource. Depends on provider. */

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~AddonDefinition();

	///----------------------------------------------------------------------------------------------------
	/// IsValid:
	/// 	Returns true if the addon has all the required fields.
	///----------------------------------------------------------------------------------------------------
	bool IsValid();

	AddonDefinition& operator=(const AddonDefinition& rhs);
};

#endif
