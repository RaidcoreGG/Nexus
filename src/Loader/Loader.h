#ifndef LOADER_H
#define LOADER_H

#include <mutex>
#include <map>
#include <unordered_map>
#include <thread>
#include <filesystem>
#include <Shlobj.h>

#include "ELoaderAction.h"
#include "Addon.h"
#include "AddonAPI.h"

namespace Loader
{
	extern std::mutex Mutex;
	extern std::unordered_map<std::filesystem::path, ELoaderAction> QueuedAddons;		/* To be loaded or unloaded addons */
	extern std::unordered_map<std::filesystem::path, Addon*> Addons;					/* Addons and their corresponding paths */
	extern std::map<int, AddonAPI*> ApiDefs;											/* Addon API definitions, created on demand */

	extern int DirectoryChangeCountdown;
	extern std::thread LoaderThread;

	extern PIDLIST_ABSOLUTE FSItemList;
	extern ULONG FSNotifierID;

	/* Registers the addon directory update notifications and loads all addons. */
	void Initialize();
	/* Deregisters the directory updates and unloads all addons. */
	void Shutdown();

	/* Returns 0 if message was processed. */
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/* Processes all currently queued addons. */
	void ProcessQueue();
	/* Pushes an item to the queue. */
	void QueueAddon(ELoaderAction aAction, const std::filesystem::path& aPath);

	/* Notifies that something in the addon directory changed. */
	void NotifyChanges();
	/* Detects and processes any changes to addons. */
	void ProcessChanges();

	/* Loads an addon. */
	void LoadAddon(const std::filesystem::path& aPath, bool aIsReload = false);
	/* Unloads an addon. */
	void UnloadAddon(const std::filesystem::path& aPath, bool aIsShutdown = false);
	/* Unloads, then uninstalls an addon. */
	void UninstallAddon(const std::filesystem::path& aPath);
	/* Unloads, then loads an addon. */
	void ReloadAddon(const std::filesystem::path& aPath);
	/* Swaps addon.dll with addon.dll.update. */
	void UpdateSwapAddon(const std::filesystem::path& aPath);
	/* Updates an addon. */
	bool UpdateAddon(const std::filesystem::path& aPath, AddonDefinition* aDefinitions);

	/* Gets or creates a pointer to the provided version, or nullptr if no such version exists. */
	AddonAPI* GetAddonAPI(int aVersion);
	/* Returns the size of the provided version. */
	long GetAddonAPISize(int aVersion);

	/* HELPER: Copies the addon definitions. */
	void CopyAddonDefs(AddonDefinition* aDefinitions, AddonDefinition** aOutDefinitions);
	/* HELPER: Frees the addon definitions. */
	void FreeAddonDefs(AddonDefinition** aDefinitions);
}

#endif