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
#include "LibraryAddon.h"
#include "AddonPreferences.h"
#include "Loader.h"
#include "Events/EventHandler.h"
#include "Services/Logging/LogHandler.h"
#include "LoaderConst.h"
#include "Consts.h"

/* prototype for cyclic dependency */
class CLoader;

///----------------------------------------------------------------------------------------------------
/// EAddonType Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonType
{
	None,
	Nexus,
	Library,
	ArcDPS,
	Incompatible
};

///----------------------------------------------------------------------------------------------------
/// AddonFlags Struct
///----------------------------------------------------------------------------------------------------
struct AddonFlags
{
	/* Behavior flags */
	bool                   IsLocked             : 1; /* Set if the state can no longer be changed. E.g. because of EAddonFlags::DisableHotloading. */

	/* Info flags */
	bool                   IsModuleLoaded       : 1; /* Set when LoadLibrary was called. */
	bool                   IsUpdateAvailable    : 1; /* Set when an update is available after checking. */
	bool                   UninstallNextLaunch  : 1; /* Set when the addon is locked and/or can't remove the file on disk. */

	/* Action flags */
	bool                   IsCheckingForUpdates : 1; /* Set when an update check is currently running. */
	bool                   IsUpdating           : 1; /* Set while the addon is updating. */
	bool                   IsLoading            : 1; /* Set when the addon is currently loading. */
	bool                   IsUnloading          : 1; /* Set when the addon is currently unloading. */
	bool                   IsInstalling         : 1; /* Set for library addons when installing. */
	bool                   IsUninstalling       : 1; /* Set when the addon is currently uninstalling. */

	/* Pref flags */
	bool                   LoadNextLaunch       : 1; /* Set when the addon is locked and should be loaded next game start. E.g. because of EAddonFlags::OnlyLoadOnGameLaunch. */
	bool                   UnloadNextLaunch     : 1; /* Set when the addon is locked and should be unloaded next game start. */
};

///----------------------------------------------------------------------------------------------------
/// EAddonAction Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonAction
{
	None,
	Load,
	Unload,
	Install,
	Uninstall,
	CheckUpdate,
	Update
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
	CAddon(AddonDefinition* aAddonDef);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CAddon(ArcdpsPlugin* aPluginDef);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CAddon();

	///----------------------------------------------------------------------------------------------------
	/// GetFlags:
	/// 	Returns flags of the addon.
	///----------------------------------------------------------------------------------------------------
	const AddonFlags& GetFlags();

	///----------------------------------------------------------------------------------------------------
	/// IsUpdateAvailable:
	/// 	Returns true if an update is available.
	///----------------------------------------------------------------------------------------------------
	bool IsUpdateAvailable();
	
	///----------------------------------------------------------------------------------------------------
	/// GetError:
	/// 	Returns the current error message.
	///----------------------------------------------------------------------------------------------------
	std::string GetError();

	///----------------------------------------------------------------------------------------------------
	/// OwnsAddress:
	/// 	Returns true if the address is part of the addons address space.
	///----------------------------------------------------------------------------------------------------
	bool OwnsAddress(void* aAddress);

	///----------------------------------------------------------------------------------------------------
	/// GetName:
	/// 	Returns the name of the addon.
	///----------------------------------------------------------------------------------------------------
	std::string GetName();

	///----------------------------------------------------------------------------------------------------
	/// QueueAction:
	/// 	Queues and performs an action.
	///----------------------------------------------------------------------------------------------------
	void QueueAction(EAddonAction aAction);

	private:
	CLoader*                   Loader;
	CEventApi*                 EventApi;
	CLogHandler*               Logger;

	EAddonAction               QueuedAction;
	std::condition_variable    ConVar;
	bool                       IsCanceled;
	std::mutex                 ProcessorMutex;
	std::thread                ProcessorThread;

	std::mutex                 Mutex;
	EAddonType                 Type;
	EAddonState                State;
	AddonFlags                 Flags;
	std::string                ErrorDescription;

	AddonPreferences*          Preferences;
	LibraryAddon*              LibraryDef;       /* Used when the addon is available for download. */

	union /* Definitions */
	{
		AddonDefinition        AddonDef;         /* Used for Nexus addons. */
		ArcdpsPlugin           PluginDef;        /* Used for ArcDPS plugins. */
	};
	std::filesystem::path      Location;         /* Contains the intended location. */
	std::filesystem::path      RealLocation;     /* Contains the real location. E.g. .tmp path during updates. */
	std::vector<unsigned char> MD5;
	HMODULE                    Module;
	size_t                     ModuleSize;

	///----------------------------------------------------------------------------------------------------
	/// Process:
	/// 	Performs queued actions.
	///----------------------------------------------------------------------------------------------------
	void Process();

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
	/// Update:
	/// 	Updates the addon.
	///----------------------------------------------------------------------------------------------------
	void Update();

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
