///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonListing.h
/// Description  :  Addon listing UI wrapper.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef UI_ADDONLISTING_H
#define UI_ADDONLISTING_H

#include <string>
#include <unordered_map>

#include "Core/Addons/Addon.h"
#include "Core/Addons/Library/LibAddon.h"
#include "UI/DisplayBinds.h"
#include "UI/FuncDefs.h"

using ArcExtensionDef_t = ArcDPS::ExtensionDef_t;

struct AddonListing_t
{
	bool                                               HasNexusDef;
	bool                                               HasArcDef;
	bool                                               HasLibDef;

	CAddon*                                            Addon;
	AddonDefV1_t                                       NexusAddonDefV1;
	ArcExtensionDef_t                                  ArcExtensionDef;
	LibraryAddon_t                                     LibraryDef;

	std::string                                        GithubURL;

	std::unordered_map<std::string, InputBindPacked_t> InputBinds;
	GUI_RENDER                                         OptionsRender;
	bool                                               IsHovered;
	bool                                               IsInstalling;

	inline uint32_t GetSig() const
	{
		if (this->HasNexusDef)
		{
			return this->NexusAddonDefV1.Signature;
		}

		if (this->HasArcDef)
		{
			return this->ArcExtensionDef.Signature;
		}

		if (this->HasLibDef)
		{
			return this->LibraryDef.Signature;
		}

		return 0;
	}

	inline std::string GetName() const
	{
		if (this->HasNexusDef)
		{
			return this->NexusAddonDefV1.Name;
		}

		if (this->HasArcDef)
		{
			return this->ArcExtensionDef.Name;
		}

		if (this->HasLibDef)
		{
			return this->LibraryDef.Name;
		}

		return "";
	}

	inline std::string GetAuthor() const
	{
		if (this->HasNexusDef)
		{
			return this->NexusAddonDefV1.Author;
		}

		// Arc Extensions don't have authors.

		if (this->HasLibDef)
		{
			return this->LibraryDef.Author;
		}

		return "";
	}

	inline std::string GetDesc() const
	{
		if (this->HasNexusDef)
		{
			return this->NexusAddonDefV1.Description;
		}

		// Arc Extensions don't have descriptions.

		if (this->HasLibDef)
		{
			return this->LibraryDef.Description;
		}

		return "";
	}

	inline std::string GetVersion() const
	{
		if (this->HasNexusDef)
		{
			return this->NexusAddonDefV1.Version.string();
		}

		if (this->HasArcDef)
		{
			return this->ArcExtensionDef.Build;
		}

		return "";
	}
};

#endif
