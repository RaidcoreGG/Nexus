///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.h
/// Description  :  Handles addon hot-loading, updates etc.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADER_H
#define LOADER_H

#include <mutex>
#include <map>
#include <vector>
#include <unordered_map>
#include <thread>
#include <filesystem>
#include <Shlobj.h>
#include <condition_variable>

#include "ELoaderAction.h"
#include "Addon.h"
#include "API/AddonAPI.h"

#include "Loader/NexusLinkData.h"

namespace Loader
{
	extern NexusLinkData*				NexusLink;

	extern std::mutex					Mutex;
	extern std::unordered_map<
		std::filesystem::path,
		ELoaderAction
	>									QueuedAddons;					/* To be loaded or unloaded addons */
	extern std::vector<Addon*>			Addons;
	extern bool							HasCustomConfig;

	extern int							DirectoryChangeCountdown;
	extern std::condition_variable		ConVar;
	extern std::mutex					ThreadMutex;
	extern std::thread					LoaderThread;
	extern bool							IsSuspended;

	extern PIDLIST_ABSOLUTE				FSItemList;
	extern ULONG						FSNotifierID;

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Registers the addon directory update notifications and loads all addons.
	///----------------------------------------------------------------------------------------------------
	void Initialize();

	///----------------------------------------------------------------------------------------------------
	/// Shutdown:
	/// 	Deregisters the directory updates and calls unload on all addons.
	///----------------------------------------------------------------------------------------------------
	void Shutdown();

	///----------------------------------------------------------------------------------------------------
	/// LoadAddonConfig:
	/// 	Load AddonConfig.
	///----------------------------------------------------------------------------------------------------
	void LoadAddonConfig();

	///----------------------------------------------------------------------------------------------------
	/// SaveAddonConfig:
	/// 	Save AddonConfig.
	///----------------------------------------------------------------------------------------------------
	void SaveAddonConfig();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// ProcessQueue:
	/// 	Processes all currently queued addons.
	///----------------------------------------------------------------------------------------------------
	void ProcessQueue();

	///----------------------------------------------------------------------------------------------------
	/// QueueAddon:
	/// 	Pushes an item to the queue.
	///----------------------------------------------------------------------------------------------------
	void QueueAddon(ELoaderAction aAction, const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// NotifyChanges:
	/// 	Notifies that something in the addon directory changed.
	///----------------------------------------------------------------------------------------------------
	void NotifyChanges();

	///----------------------------------------------------------------------------------------------------
	/// ProcessChanges:
	/// 	Detects and processes any changes to addons.
	///----------------------------------------------------------------------------------------------------
	void ProcessChanges();

	///----------------------------------------------------------------------------------------------------
	/// LoadAddon:
	/// 	Loads an addon.
	///----------------------------------------------------------------------------------------------------
	void LoadAddon(const std::filesystem::path& aPath, bool aIsReload = false);
	
	///----------------------------------------------------------------------------------------------------
	/// UnloadAddon:
	/// 	Unloads an addon and performs a reload as soon as the addon returns, if requested.
	///----------------------------------------------------------------------------------------------------
	void UnloadAddon(const std::filesystem::path& aPath, bool aDoReload = false);

	///----------------------------------------------------------------------------------------------------
	/// FreeAddon:
	/// 	Calls FreeLibrary on the specified addon.
	/// 	This function should not be invoked manually, but through Addon::Unload + Queue(Free).
	///----------------------------------------------------------------------------------------------------
	void FreeAddon(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// UninstallAddon:
	/// 	Uninstalls an addon, or moves it to addon.dll.uninstall to be cleaned up by the loader later.
	/// 	This function should not be invoked manually, but through Unload + FollowUpAction::Uninstall.
	///----------------------------------------------------------------------------------------------------
	void UninstallAddon(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// UpdateSwapAddon:
	/// 	Swaps addon.dll with addon.dll.update.
	/// 	Returns true if there was an update dll.
	///----------------------------------------------------------------------------------------------------
	bool UpdateSwapAddon(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetOwner:
	/// 	Returns the name of the addon owning the provided address.
	///----------------------------------------------------------------------------------------------------
	std::string GetOwner(void* aAddress);

	///----------------------------------------------------------------------------------------------------
	/// FindAddonBySig:
	/// 	Returns the addon with a matching signature or nullptr.
	///----------------------------------------------------------------------------------------------------
	Addon* FindAddonBySig(signed int aSignature);

	///----------------------------------------------------------------------------------------------------
	/// FindAddonByPath:
	/// 	Returns the addon with a matching path or nullptr.
	///----------------------------------------------------------------------------------------------------
	Addon* FindAddonByPath(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// FindAddonByMatchSig:
	/// 	Returns the addon with a matching mock signature or nullptr.
	///----------------------------------------------------------------------------------------------------
	Addon* FindAddonByMatchSig(signed int aMatchSignature);

	///----------------------------------------------------------------------------------------------------
	/// FindAddonByMD5:
	/// 	Returns the addon with a matching MD5 or nullptr.
	///----------------------------------------------------------------------------------------------------
	Addon* FindAddonByMD5(std::vector<unsigned char> aMD5);

	///----------------------------------------------------------------------------------------------------
	/// SortAddons:
	/// 	Sorts addons by name but prioritizes DUU state.
	///----------------------------------------------------------------------------------------------------
	void SortAddons();

	///----------------------------------------------------------------------------------------------------
	/// GetGameBuild:
	/// 	Gets the game build and sets the Disable Until Update state.
	///----------------------------------------------------------------------------------------------------
	void GetGameBuild();
}

#endif
