#ifndef LOADER_H
#define LOADER_H

#include <mutex>
#include <map>
#include <unordered_map>
#include <thread>
#include <filesystem>

#include "ELoaderAction.h"
#include "Addon.h"
#include "AddonAPI.h"

namespace Loader
{
	extern std::mutex Mutex;
	extern std::unordered_map<std::filesystem::path, ELoaderAction> QueuedAddons;		/* To be loaded or unloaded addons */
	extern std::unordered_map<std::filesystem::path, Addon*> Addons;					/* Addons and their corresponding paths */
	extern std::map<int, AddonAPI*> ApiDefs;								/* Addon API definitions, created on demand */

	extern std::thread LoaderThread;

	/* Initializes the Loader and the API. */
	void Initialize();
	/* Shuts the Loader down and frees all addons. */
	void Shutdown();

	/* Processes all currently queued addons. */
	void ProcessQueue();
	/* Pushes an item to the queue. */
	void QueueAddon(ELoaderAction aAction, std::filesystem::path aPath);

	/* Loads an addon. */
	void LoadAddon(std::filesystem::path aPath);
	/* Unloads an addon. */
	void UnloadAddon(std::filesystem::path aPath);
	/* Unloads, then uninstalls an addon. */
	void UninstallAddon(std::filesystem::path aPath);
	/* Detects if there are addons that are not currently loaded, or if loaded addons have been removed. */
	void DetectAddonsLoop();

	AddonAPI* GetAddonAPI(int aVersion);
	long GetAddonAPISize(int aVersion);
}

#endif