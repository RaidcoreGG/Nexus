///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonDefV1.h
/// Description  :  Configuration for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDONDEFV1_H
#define ADDONDEFV1_H

#include <cstdint>

#include "Core/Addons/AddEnum.h"
#include "Core/Addons/API/ApiBase.h"
#include "Core/Addons/Definitions/DefEnum.h"
#include "Core/Versioning/VerU64_4XS16.h"

struct AddonDefRawV1_t;

typedef AddonDefRawV1_t* (*GETADDONDEF) ();
typedef void             (*ADDON_LOAD)  (AddonAPI_t* aAPI);
typedef void             (*ADDON_UNLOAD)();

///----------------------------------------------------------------------------------------------------
/// AddonDefRawV1_t Struct
/// 	Used for API Revisions 1-6.
///----------------------------------------------------------------------------------------------------
struct AddonDefRawV1_t
{
	/* Required */
	uint32_t        Signature;   /* Unique addon identifier.                                                */
	uint32_t        APIVersion;  /* Which API revision to pass to the load function. Use NEXUS_API_VERSION. */
	const char*     Name;        /* Name of the addon as shown in the library.                              */
	VerU64_4XS16_t  Version;
	const char*     Author;      /* Author of the addon.                                                    */
	const char*     Description; /* Short description.                                                      */
	ADDON_LOAD      Load;        /* Load function.                                                          */
	ADDON_UNLOAD    Unload;      /* Unload function. Optional, if EAddonFlags::DisableHotloading is set.    */
	EAddonDefFlags  Flags;       /* Additional flags, to modify behavior or enable features.                */

	/* Optional */
	EUpdateProvider Provider;    /* How to check for updates.                                               */
	const char*     UpdateLink;  /* Link to the update resource.                                            */

	///----------------------------------------------------------------------------------------------------
	/// HasMinimumRequirements:
	/// 	Returns true, if the addon fulfills the minimum requirements.
	///----------------------------------------------------------------------------------------------------
	inline bool HasMinimumRequirements()
	{
		if (this->Signature != 0 &&
			this->Name &&
			this->Author &&
			this->Description &&
			this->Load &&
			((bool)(this->Flags & EAddonDefFlags::DisableHotloading) || this->Unload))
		{
			return true;
		}

		return false;
	}
};

///----------------------------------------------------------------------------------------------------
/// AddonDefV1_t Struct
/// 	Used for API Revisions 1-6.
///----------------------------------------------------------------------------------------------------
struct AddonDefV1_t
{
	/* Required */
	uint32_t                  Signature;   /* Unique addon identifier.                                                */
	uint32_t                  APIVersion;  /* Which API revision to pass to the load function. Use NEXUS_API_VERSION. */
	std::string               Name;        /* Name of the addon as shown in the library.                              */
	MajorMinorBuildRevision_t Version;
	std::string               Author;      /* Author of the addon.                                                    */
	std::string               Description; /* Short description.                                                      */
	ADDON_LOAD                Load;        /* Load function.                                                          */
	ADDON_UNLOAD              Unload;      /* Unload function. Optional, if EAddonFlags::DisableHotloading is set.    */
	EAddonDefFlags            Flags;       /* Additional flags, to modify behavior or enable features.                */

	/* Optional */
	EUpdateProvider           Provider;    /* How to check for updates.                                               */
	std::string               UpdateLink;  /* Link to the update resource.                                            */

	AddonDefV1_t() = default;
	inline AddonDefV1_t(AddonDefRawV1_t aRawDef)
	{
		this->Signature = aRawDef.Signature;
		this->APIVersion = aRawDef.APIVersion;
		this->Name = aRawDef.Name
			? aRawDef.Name
			: "";
		this->Version = aRawDef.Version;
		this->Author = aRawDef.Author
			? aRawDef.Author
			: "";
		this->Description = aRawDef.Description
			? aRawDef.Description
			: "";
		this->Load = aRawDef.Load;
		this->Unload = aRawDef.Unload;
		this->Flags = aRawDef.Flags;
		this->Provider = aRawDef.Provider;
		this->UpdateLink = aRawDef.UpdateLink
			? aRawDef.UpdateLink
			: "";
	}
};

#endif
