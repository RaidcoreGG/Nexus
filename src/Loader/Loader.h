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

#include "AddonDefinition.h"
#include "LoadedAddon.h"
#include "FuncDefs.h"

#include "../Mumble/LinkedMem.h"
#include "../Logging/LogHandler.h"
#include "../Events/EventHandler.h"
#include "../Keybinds/KeybindHandler.h"
#include "../imgui/imgui.h"
#include "../minhook/mh_hook.h"
#include "../DataLink/DataLink.h"
#include "../Textures/TextureLoader.h"
#include "../GUI/Widgets/QuickAccess/QuickAccess.h"

namespace Loader
{
	extern std::mutex Mutex;
	extern std::map<std::filesystem::path, LoadedAddon> AddonDefs;		/* Loaded Addons and their corresponding paths */
	extern AddonAPI APIDef;

	extern std::thread UpdateThread;

	extern std::set<std::filesystem::path> Blacklist;					/* Blacklisted files which won't be reloaded */

	void Initialize();
	void Shutdown();

	void LoadAddon(std::filesystem::path aPath);
	void UnloadAddon(std::filesystem::path aPath);

	void VerifyCallbacks(void* aStartAddress, void* aEndAddress);

	void DetectAddons();
}

#endif