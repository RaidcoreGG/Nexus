#ifndef LOADER_H
#define LOADER_H

#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>
#include <Windows.h>
#include <Psapi.h>

#include "../core.h"
#include "../State.h"
#include "../Shared.h"
#include "../Paths.h"
#include "../Renderer.h"

#include "ELoaderAction.h"
#include "AddonDefinition.h"
#include "Addon.h"
#include "FuncDefs.h"

#include "../Mumble/LinkedMem.h"
#include "../Logging/LogHandler.h"
#include "../Events/EventHandler.h"
#include "../WndProc/WndProcHandler.h"
#include "../Keybinds/KeybindHandler.h"
#include "../imgui/imgui.h"
#include "../minhook/mh_hook.h"
#include "../DataLink/DataLink.h"
#include "../Textures/TextureLoader.h"
#include "../GUI/GUI.h"
#include "../GUI/Widgets/QuickAccess/QuickAccess.h"

namespace Loader
{
	extern std::mutex Mutex;
	extern std::map<std::filesystem::path, ELoaderAction> QueuedAddons;		/* To be loaded or unloaded addons */
	extern std::map<std::filesystem::path, Addon*> Addons;					/* Addons and their corresponding paths */
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