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
#include "Core/Versioning/MajorMinorBuildRevision.h"

struct AddonDefV1_t;

typedef AddonDefV1_t* (*GETADDONDEF_V1) ();
typedef void          (*ADDON_LOAD)     (AddonAPI_t* aAPI);
typedef void          (*ADDON_UNLOAD)   ();

typedef MajorMinorBuildRevision_t AddonVersion_t;

///----------------------------------------------------------------------------------------------------
/// AddonDefV1_t Struct
/// 	Used for API Revisions 1-6.
///----------------------------------------------------------------------------------------------------
struct AddonDefV1_t
{
	/* Required */
	uint32_t        Signature;   /* Unique addon identifier.                                                */
	uint32_t        APIVersion;  /* Which API revision to pass to the load function. Use NEXUS_API_VERSION. */
	const char*     Name;        /* Name of the addon as shown in the library.                              */
	AddonVersion_t  Version;
	const char*     Author;      /* Author of the addon.                                                    */
	const char*     Description; /* Short description.                                                      */
	ADDON_LOAD      Load;        /* Load function.                                                          */
	ADDON_UNLOAD    Unload;      /* Unload function. Optional, if Flags::DisableHotloading is set.          */
	EAddonDefFlags  Flags;       /* Additional flags, to modify behavior or enable features.                */

	/* Optional */
	EUpdateProvider Provider;    /* How to check for updates.                                               */
	const char*     UpdateLink;  /* Link to the update resource.                                            */

	inline std::string GetName()
	{
		return this->Name ? this->Name : "";
	}

	inline std::string GetAuthor()
	{
		return this->Author ? this->Author : "";
	}

	inline std::string GetDescription()
	{
		return this->Description ? this->Description : "";
	}

	inline std::string GetUpdateLink()
	{
		return this->UpdateLink ? this->UpdateLink : "";
	}

	///----------------------------------------------------------------------------------------------------
	/// ctor (copy)
	///----------------------------------------------------------------------------------------------------
	inline AddonDefV1_t(const AddonDefV1_t& aOther)
	{
		this->Signature   = aOther.Signature;
		this->APIVersion  = aOther.APIVersion;
		this->Name        = nullptr;
		this->Version     = aOther.Version;
		this->Author      = nullptr;
		this->Description = nullptr;
		this->Load        = aOther.Load;
		this->Unload      = aOther.Unload;
		this->Flags       = aOther.Flags;
		this->Provider    = aOther.Provider;
		this->UpdateLink  = nullptr;

		if (aOther.Name && aOther.Name[0])
		{
			this->Name = _strdup(aOther.Name);
		}

		if (aOther.Author && aOther.Author[0])
		{
			this->Author = _strdup(aOther.Author);
		}

		if (aOther.Description && aOther.Description[0])
		{
			this->Description = _strdup(aOther.Description);
		}

		if (aOther.UpdateLink && aOther.UpdateLink[0])
		{
			this->UpdateLink = _strdup(aOther.UpdateLink);
		}
	}

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	inline ~AddonDefV1_t()
	{
		if (this->Name)
		{
			free((void*)this->Name);
		}

		if (this->Author)
		{
			free((void*)this->Author);
		}

		if (this->Description)
		{
			free((void*)this->Description);
		}

		if (this->UpdateLink)
		{
			free((void*)this->UpdateLink);
		}
	}

	///----------------------------------------------------------------------------------------------------
	/// HasMinimumRequirements:
	/// 	Returns true, if the addon fulfills the minimum requirements.
	///----------------------------------------------------------------------------------------------------
	inline bool HasMinimumRequirements()
	{
		bool hasUnloadOrDisablesHotloading
			= (this->Flags & EAddonDefFlags::DisableHotloading) == EAddonDefFlags::DisableHotloading
			|| this->Unload;

		if (this->Signature
			&& this->Name
			&& this->Author
			&& this->Description
			&& this->Load
			&& hasUnloadOrDisablesHotloading)
		{
			return true;
		}

		return false;
	}
};

#endif
