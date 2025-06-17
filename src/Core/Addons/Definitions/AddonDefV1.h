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

#include "Core/Versioning/VerU64_4XS16.h"
#include "Engine/Loader/LdrEnum.h"
#include "Engine/Loader/LdrFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// AddonDef_t Struct
/// 	Used for API Revisions 1-6.
///----------------------------------------------------------------------------------------------------
struct AddonDef_t
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
	EAddonFlags     Flags;       /* Additional flags, to modify behavior or enable features.                */

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
			(this->HasFlag(EAddonFlags::DisableHotloading) || this->Unload))
		{
			return true;
		}

		return false;
	}

	///----------------------------------------------------------------------------------------------------
	/// HasFlag:
	/// 	Returns true, if the addon declares the given flag.
	///----------------------------------------------------------------------------------------------------
	inline bool HasFlag(EAddonFlags aAddonFlag)
	{
		return (bool)(this->Flags & aAddonFlag);
	}

	///----------------------------------------------------------------------------------------------------
	/// Copy:
	/// 	Deep copies a source to a specified out destination.
	///----------------------------------------------------------------------------------------------------
	static inline void Copy(AddonDef_t* aSrc, AddonDef_t** aDst)
	{
		if (aDst == nullptr) { return; }

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
		(*aDst)->UpdateLink = aSrc->UpdateLink
			? _strdup(aSrc->UpdateLink)
			: nullptr;
	}

	///----------------------------------------------------------------------------------------------------
	/// Free:
	/// 	Fully frees an addon definition and other resources and sets it to nullptr.
	///----------------------------------------------------------------------------------------------------
	static inline void Free(AddonDef_t** aDefinitions)
	{
		if (aDefinitions == nullptr)  { return; }
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
};

#endif
