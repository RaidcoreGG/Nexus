///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addon.h
/// Description  :  Contains the logic for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDON_H
#define ADDON_H

#include <Windows.h>
#include <vector>
#include <mutex>
#include <filesystem>

#include "arcdps/ArcDPS.h"

#include "EAddonState.h"
#include "AddonDefinition.h"
#include "EUpdateMode.h"
#include "LibraryAddon.h"

///----------------------------------------------------------------------------------------------------
/// EAddonType Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonType
{
	None,
	Nexus,
	AddonConfig,
	Library,
	ArcDPS
};

///----------------------------------------------------------------------------------------------------
/// CAddon Class
///----------------------------------------------------------------------------------------------------
class CAddon
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CAddon(signed int aSignature);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CAddon(AddonDefinition* aAddonDef);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CAddon(ArcDPS::PluginInfo* aPluginDef);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CAddon();

	///----------------------------------------------------------------------------------------------------
	/// Merge:
	/// 	Merges an addon definition into a config.
	///----------------------------------------------------------------------------------------------------
	void Merge(AddonDefinition* aAddonDef);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the addon.
	///----------------------------------------------------------------------------------------------------
	void Load();
	
	///----------------------------------------------------------------------------------------------------
	/// Unload:
	/// 	Unloads the addon.
	///----------------------------------------------------------------------------------------------------
	void Unload();
	
	///----------------------------------------------------------------------------------------------------
	/// Install:
	/// 	Installs the addon.
	///----------------------------------------------------------------------------------------------------
	void Install();

	///----------------------------------------------------------------------------------------------------
	/// Uninstall:
	/// 	Uninstalls the addon.
	///----------------------------------------------------------------------------------------------------
	void Uninstall();
	
	///----------------------------------------------------------------------------------------------------
	/// CheckForUpdate:
	/// 	Checks if an update is available.
	///----------------------------------------------------------------------------------------------------
	void CheckForUpdate();

	///----------------------------------------------------------------------------------------------------
	/// IsUpdateAvailable:
	/// 	Returns true if an update is available.
	///----------------------------------------------------------------------------------------------------
	bool IsUpdateAvailable();
	
	///----------------------------------------------------------------------------------------------------
	/// Update:
	/// 	Updates the addon.
	///----------------------------------------------------------------------------------------------------
	void Update();

	///----------------------------------------------------------------------------------------------------
	/// OwnsAddress:
	/// 	Returns true if the passed address is part of the addon's address space.
	///----------------------------------------------------------------------------------------------------
	bool OwnsAddress(void* aAddress);

	///----------------------------------------------------------------------------------------------------
	/// GetError:
	/// 	Returns the current error message.
	///----------------------------------------------------------------------------------------------------
	std::string GetError();

	///----------------------------------------------------------------------------------------------------
	/// IsPersistent:
	/// 	Returns true if the addon should store data.
	///----------------------------------------------------------------------------------------------------
	bool IsPersistent();

	private:
	std::mutex                 Mutex;
	EAddonType                 Type;
	EAddonState                State;
	struct
	{
		/* Behavior flags */
		bool                   IsLocked             : 1; /* Set if the addon declares EAddonFlags::DisableHotloading. */

		/* Update flags*/
		bool                   IsCheckingForUpdates : 1; /* Set when an update check is currently running. */
		bool                   IsUpdateAvailable    : 1; /* Set when an update is available after checking. */

		/* Transition flags */
		bool                   IsUpdating           : 1; /* Set while the addon is updating. */
		bool                   IsDisabling          : 1; /* Set when the addon is currently unloading. */
		bool                   IsInstalling         : 1; /* Set for library addons when installing. */
		bool                   IsUninstalling       : 1; /* Set when the addon is currently uninstalling. */

		/* Restart flags */
		bool                   EnableNextLaunch     : 1; /* Set when the addon is locked and should enable next game start. E.g. because of EAddonFlags::OnlyLoadOnGameLaunch. */
		bool                   DisableNextLaunch    : 1; /* Set when the addon is locked and should disable next game start. */
		bool                   UninstallNextLaunch  : 1; /* Set when the addon is locked and/or can't remove the file on disk. */
	} Flags;

	struct
	{
		EUpdateMode            UpdateMode;               /* Behavior regarding updates. */
		bool                   AllowPreReleases;         /* If the update provider supports pre-releases. */
		bool                   IsFavorite;               /* Marked as favorite. */
		bool                   IsDisabledUntilUpdate;    /* Overrides UpdateMode::Disable to prompt instead. */
	} Preferences;

	std::filesystem::path      Location;                 /* Contains the intended location. */
	std::filesystem::path      RealLocation;             /* Contains the real location. E.g. .tmp path during updates. */
	std::vector<unsigned char> MD5;
	HMODULE                    Module;
	size_t                     ModuleSize;

	std::string                ErrorDescription;

	union /* Definitions */
	{
		AddonDefinition        AddonDef;                 /* Used for Nexus addons. */
		signed int             Signature;                /* Used when the addon is initialized from config. */
		ArcDPS::PluginInfo     PluginDef;                /* Used for ArcDPS plugins. */
	};
	LibraryAddon*              LibraryDef;               /* Used when the addon is available for download. */

	///----------------------------------------------------------------------------------------------------
	/// Free:
	/// 	Calls free library and frees other resources.
	///----------------------------------------------------------------------------------------------------
	void Free();

	///----------------------------------------------------------------------------------------------------
	/// ShouldUpdate:
	/// 	Returns true if the addon should update.
	///----------------------------------------------------------------------------------------------------
	bool ShouldUpdate();

	///----------------------------------------------------------------------------------------------------
	/// ShouldLoad:
	/// 	Returns true if the addon should load.
	///----------------------------------------------------------------------------------------------------
	bool ShouldLoad();
};

#endif
