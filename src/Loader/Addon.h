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
#include "EAddonInterface.h"
#include "Events/EventHandler.h"
#include "Services/Logging/LogHandler.h"
#include "LoaderConst.h"
#include "Consts.h"

/* prototype for cyclic dependency */
class CLoader;

///----------------------------------------------------------------------------------------------------
/// AddonFlags Struct
///----------------------------------------------------------------------------------------------------
struct AddonFlags
{
	/* Behavior flags */
	bool IsStateLocked     : 1; /* State can or must not be modified. */
	bool IsModuleLocked    : 1; /* Module cannot be unloaded. */
	bool IsUninstalled     : 1; /* Set when the addon is uninstalled, but can't be removed at runtime due to locked file/module. */
	bool IsAPICommitted    : 1; /* API calls can be instantly registered. */

	/* Info flags */
	bool IsModuleLoaded    : 1; /* Set when LoadLibrary was called. */
	bool IsUpdateAvailable : 1; /* Set when an update is available after checking. */

	bool HasLibraryDef     : 1;
	bool HasAddonDef       : 1;
	bool HasPluginDef      : 1;

	/* Action flags */
	bool IsRunningAction   : 1; /* Set when any action is running. */

	/* Pref modifying flags */
	bool LoadNextLaunch    : 1; /* Set when the addon is locked and should be loaded next game start. E.g. because of EAddonFlags::OnlyLoadOnGameLaunch. */
	bool UnloadNextLaunch  : 1; /* Set when the addon is locked and should be unloaded next game start. */
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
	CAddon(CLogHandler* aLogger, CEventApi* aEventApi, CPreferenceMgr* aPrefMgr, CLibraryMgr* aLibraryMgr);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CAddon();

	///----------------------------------------------------------------------------------------------------
	/// IsValid:
	/// 	Returns true, if the addon has at least one of LibraryDef, AddonDef or PluginDef.
	///----------------------------------------------------------------------------------------------------
	bool IsValid() const;

	///----------------------------------------------------------------------------------------------------
	/// GetFlags:
	/// 	Returns flags of the addon.
	///----------------------------------------------------------------------------------------------------
	const AddonFlags& GetFlags() const;

	///----------------------------------------------------------------------------------------------------
	/// IsUpdateAvailable:
	/// 	Returns true if an update is available.
	///----------------------------------------------------------------------------------------------------
	bool IsUpdateAvailable() const;
	
	///----------------------------------------------------------------------------------------------------
	/// GetError:
	/// 	Returns the current error message.
	///----------------------------------------------------------------------------------------------------
	std::string GetError();

	///----------------------------------------------------------------------------------------------------
	/// OwnsAddress:
	/// 	Returns true if the address is part of the addons address space.
	///----------------------------------------------------------------------------------------------------
	bool OwnsAddress(void* aAddress) const;

	///----------------------------------------------------------------------------------------------------
	/// GetName:
	/// 	Returns the name of the addon.
	///----------------------------------------------------------------------------------------------------
	const std::string& GetName() const;

	///----------------------------------------------------------------------------------------------------
	/// GetLocation:
	/// 	Returns the location of the addon.
	///----------------------------------------------------------------------------------------------------
	const std::filesystem::path& GetLocation() const;

	///----------------------------------------------------------------------------------------------------
	/// GetMD5:
	/// 	Returns the MD5 of the addon.
	///----------------------------------------------------------------------------------------------------
	const std::vector<unsigned char>& GetMD5() const;

	///----------------------------------------------------------------------------------------------------
	/// GetLibraryDef:
	/// 	Returns the library definition of the addon.
	///----------------------------------------------------------------------------------------------------
	const LibraryAddon* GetLibraryDef() const;

	///----------------------------------------------------------------------------------------------------
	/// QueueAction:
	/// 	Queues and performs an action.
	///----------------------------------------------------------------------------------------------------
	void QueueAction(EAddonAction aAction);

	private:
	CLogHandler*               Logger;
	CEventApi*                 EventApi;
	CPreferenceMgr*            PrefMgr;
	CLibraryMgr*               LibraryMgr;

	EAddonAction               QueuedAction;
	std::condition_variable    ConVar;
	bool                       IsCanceled;
	std::mutex                 ProcessorMutex;
	std::thread                ProcessorThread;
	uint32_t                   ProcessorThreadId;

	std::mutex                 Mutex;
	EAddonState                State;
	AddonFlags                 Flags;
	std::string                ErrorDescription;

	AddonPrefs*                Preferences;
	LibraryAddon*              LibraryDef;
	AddonDefinition            AddonDef;
	ArcdpsPlugin               PluginDef;

	std::filesystem::path      Location;
	std::vector<unsigned char> MD5;
	HMODULE                    Module;
	size_t                     ModuleSize;

	///----------------------------------------------------------------------------------------------------
	/// ProcessAction:
	/// 	Performs queued actions.
	///----------------------------------------------------------------------------------------------------
	void ProcessAction();

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
	/// API_Commit:
	/// 	Commits any registrations made to API and enables instant committing.
	///----------------------------------------------------------------------------------------------------
	void API_Commit();

	///----------------------------------------------------------------------------------------------------
	/// API_Invalidate:
	/// 	Disables any registrations made to the API.
	///----------------------------------------------------------------------------------------------------
	void API_Invalidate();

	///----------------------------------------------------------------------------------------------------
	/// API_Clear:
	/// 	Clears all registrations made to the API.
	///----------------------------------------------------------------------------------------------------
	void API_Clear();

	///----------------------------------------------------------------------------------------------------
	/// ShouldCheckForUpdate:
	/// 	Returns true if the addon should check for updates.
	///----------------------------------------------------------------------------------------------------
	bool ShouldCheckForUpdate();

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
