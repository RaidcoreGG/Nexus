#include "Loader.h"

namespace Loader
{
	std::mutex Mutex;
	std::map<std::filesystem::path, LoadedAddon> AddonDefs;
	AddonAPI APIDef{};

	std::thread UpdateThread;

	std::set<std::filesystem::path> Blacklist;

	std::set<std::filesystem::path> PreviousFiles;

	void Initialize()
	{
		State::AddonHost = ggState::ADDONS_LOAD;

		/* setup APIDefs */
		APIDef.SwapChain = Renderer::SwapChain;
		APIDef.ImguiContext = Renderer::GuiContext;
		APIDef.WindowWidth = &Renderer::Width;
		APIDef.WindowHeight = &Renderer::Height;

		APIDef.CreateHook = MH_CreateHook;
		APIDef.RemoveHook = MH_RemoveHook;
		APIDef.EnableHook = MH_EnableHook;
		APIDef.DisableHook = MH_DisableHook;

		APIDef.Log = LogMessageA;
		APIDef.RegisterLogger = RegisterLogger;
		APIDef.UnregisterLogger = UnregisterLogger;

		APIDef.RaiseEvent = Events::Raise;
		APIDef.SubscribeEvent = Events::Subscribe;
		APIDef.UnsubscribeEvent = Events::Unsubscribe;

		APIDef.RegisterKeybind = Keybinds::Register;
		APIDef.UnregisterKeybind = Keybinds::Unregister;

		APIDef.GetResource = DataLink::GetResource;
		APIDef.ShareResource = DataLink::ShareResource;

		APIDef.GetTexture = TextureLoader::Get;
		APIDef.LoadTextureFromFile = TextureLoader::LoadFromFile;
		APIDef.LoadTextureFromResource = TextureLoader::LoadFromResource;

		APIDef.AddShortcut = GUI::QuickAccess::AddShortcut;
		APIDef.RemoveShortcut = GUI::QuickAccess::RemoveShortcut;
		APIDef.AddSimpleShortcut = GUI::QuickAccess::AddSimpleShortcut;
		APIDef.RemoveSimpleShortcut = GUI::QuickAccess::RemoveSimpleShortcut;

		State::AddonHost = ggState::ADDONS_READY;

		UpdateThread = std::thread(DetectAddons);
		UpdateThread.detach();
	}

	void Shutdown()
	{
		State::AddonHost = ggState::ADDONS_SHUTDOWN;
	}

	void LoadAddon(std::filesystem::path aPath)
	{
		std::string pathStr = aPath.string();
		const char* path = pathStr.c_str();
		GETADDONDEF getAddonDef = 0;
		HMODULE hMod = LoadLibraryA(path);

		/* lib load failed */
		if (!hMod)
		{
			LogDebug("Loader", "Failed LoadLibrary on \"%s\". Added to blacklist.", path);
			Blacklist.insert(aPath);
			return;
		}

		getAddonDef = (GETADDONDEF)GetProcAddress(hMod, "GetAddonDef");
		if (getAddonDef == 0)
		{
			LogDebug("Loader", "\"%s\" is not a Nexus-compatible library. Added to blacklist.", path);
			Blacklist.insert(aPath);
			FreeLibrary(hMod);
			return;
		}

		AddonDefinition* defs = getAddonDef();
		if (hMod && !defs->HasMinimumRequirements())
		{
			LogWarning("Loader", "\"%s\" does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as Load and Unload functions. Added to blacklist.", path);
			Blacklist.insert(aPath);
			FreeLibrary(hMod);
			return;
		}

		MODULEINFO moduleInfo;
		GetModuleInformation(GetCurrentProcess(), hMod, &moduleInfo, sizeof(moduleInfo));

		LoadedAddon addon{ hMod, moduleInfo.SizeOfImage, defs };

		AddonDefs.insert({ aPath, addon });

		addon.Definitions->Load(APIDef);

		LogInfo("Loader", "Loaded addon: %s", path);
	}

	void UnloadAddon(std::filesystem::path aPath)
	{
		std::string path = aPath.string();

		/* cache name for warning message and already release defs */
		std::string name = AddonDefs[aPath].Definitions->Name;

		AddonDefs[aPath].Definitions->Unload();

		if (AddonDefs[aPath].Module)
		{
			if (!FreeLibrary(AddonDefs[aPath].Module))
			{
				LogWarning("Loader", "Couldn't unload \"%s\". FreeLibrary() call failed. (%s)", name.c_str(), path.c_str());
				return;
			}
		}

		AddonDefs[aPath].Definitions = nullptr;

		/* Verify all APIs don't have any unreleased references to the addons address space */
		int leftoverRefs = 0;
		leftoverRefs += Events::Verify(AddonDefs[aPath].Module, AddonDefs[aPath].Module + AddonDefs[aPath].ModuleSize);
		leftoverRefs += GUI::QuickAccess::Verify(AddonDefs[aPath].Module, AddonDefs[aPath].Module + AddonDefs[aPath].ModuleSize);
		leftoverRefs += Keybinds::Verify(AddonDefs[aPath].Module, AddonDefs[aPath].Module + AddonDefs[aPath].ModuleSize);

		if (leftoverRefs > 0)
		{
			LogWarning("Loader", "Removed %d unreleased references from \"%s\".", leftoverRefs, name.c_str());
		}

		AddonDefs[aPath].Module = nullptr;
		AddonDefs[aPath] = {};

		AddonDefs.erase(aPath);

		LogInfo("Loader", "Unloaded addon: %s", path.c_str());
	}

	void DetectAddons()
	{
		for (;;)
		{
			if (State::AddonHost == ggState::ADDONS_SHUTDOWN) { return; }

			std::set<std::filesystem::path> onDisk;
			
			/* get which files are currently on disk */
			for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
			{
				onDisk.insert(entry.path());
			}

			/* files have changed, check what needs to be loaded and what needs to be unloaded */
			if (onDisk != PreviousFiles)
			{
				std::set<std::filesystem::path> onDiskLeft = onDisk;

				/* first check all the currently loaded addons, if they are still present */
				std::set <std::filesystem::path> unload;

				Mutex.lock();
				for (const auto& [path, addon] : Loader::AddonDefs)
				{
					if (onDiskLeft.find(path) != onDiskLeft.end())
					{
						/* addon is still present on disk, remove from the items that need to be checked */
						onDiskLeft.erase(path);
					}
					else
					{
						/* addon is no longer on disk -> unload afterwards, we can't modify this array */
						unload.insert(path);
					}
				}
				for (std::filesystem::path path : unload)
				{
					UnloadAddon(path);
				}
				Mutex.unlock();

				/* next, check if these files are on the blacklist */
				std::set <std::filesystem::path> unblacklist;

				Mutex.lock();
				for (std::filesystem::path path : Blacklist)
				{
					if (onDiskLeft.find(path) != onDiskLeft.end())
					{
						/* path is present on blacklist, remove from the items that need to be checked */
						onDiskLeft.erase(path);
					}
					else
					{
						/* no longer exists, remove from blacklist */
						unblacklist.insert(path);
					}
				}
				for (std::filesystem::path path : unblacklist)
				{
					Blacklist.erase(path);
				}
				Mutex.unlock();

				/* check if there are any files left on the disk that have not been checked, if yes try to load them */
				std::string dll = ".dll";

				for (std::filesystem::path path : onDiskLeft)
				{
					std::string pathStr = path.string();

					if (pathStr.size() >= dll.size() && 0 == pathStr.compare(pathStr.size() - dll.size(), dll.size(), dll))
					{
						Mutex.lock();
						LoadAddon(path);
						Mutex.unlock();
					}
				}
			}

			PreviousFiles = onDisk;

			Sleep(5000);
		}
	}
}