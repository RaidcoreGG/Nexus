#include "Loader.h"

namespace Loader
{
	std::mutex Mutex;
	std::vector<QueuedAddon> QueuedAddons;
	std::map<std::filesystem::path, ActiveAddon> AddonDefs;
	std::map<int, AddonAPI*> ApiDefs;

	std::thread UpdateThread;

	std::set<std::filesystem::path> Blacklist;
	std::set<std::filesystem::path> PreviousFiles;

	void Initialize()
	{
		State::AddonHost = ENexusState::ADDONS_LOAD;

		State::AddonHost = ENexusState::ADDONS_READY;

		UpdateThread = std::thread(DetectAddonsLoop);
		UpdateThread.detach();
	}
	void Shutdown()
	{
		State::AddonHost = ENexusState::ADDONS_SHUTDOWN;

		Mutex.lock();
		{
			while (AddonDefs.size() != 0)
			{
				UnloadAddon(AddonDefs.begin()->first);
			}
		}
		Mutex.unlock();
	}

	void LoadAddon(std::filesystem::path aPath)
	{
		std::string pathStr = aPath.string();
		const char* path = pathStr.c_str();
		GETADDONDEF getAddonDef = 0;
		HMODULE hMod = LoadLibraryA(path);

		/* load lib failed */
		if (!hMod)
		{
			LogDebug(CH_LOADER, "Failed LoadLibrary on \"%s\". Added to blacklist.", path);
			Blacklist.insert(aPath);
			return;
		}

		if (FindFunction(hMod, &getAddonDef, "GetAddonDef") == false)
		{
			LogDebug(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Added to blacklist.", path);
			Blacklist.insert(aPath);
			FreeLibrary(hMod);
			return;
		}

		AddonDefinition* defs = getAddonDef();
		if (hMod && !defs->HasMinimumRequirements())
		{
			LogWarning(CH_LOADER, "\"%s\" does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as Load and Unload functions. Added to blacklist.", path);
			Blacklist.insert(aPath);
			FreeLibrary(hMod);
			return;
		}

		MODULEINFO moduleInfo;
		GetModuleInformation(GetCurrentProcess(), hMod, &moduleInfo, sizeof(moduleInfo));

		ActiveAddon addon{ hMod, moduleInfo.SizeOfImage, defs };

		AddonAPI* api = GetAddonAPI(addon.Definitions->APIVersion);

		// if no addon api was requested
		// else if the requested addon api exists
		// else invalid addon, don't load
		if (addon.Definitions->APIVersion == 0)
		{
			LogInfo(CH_LOADER, "Loaded addon: %s [%p - %p]", path, hMod, ((PBYTE)hMod) + moduleInfo.SizeOfImage);
			addon.Definitions->Load(nullptr);
			AddonDefs.insert({ aPath, addon });
		}
		else if (api != nullptr)
		{
			LogInfo(CH_LOADER, "Loaded addon: %s [%p - %p]", path, hMod, ((PBYTE)hMod) + moduleInfo.SizeOfImage);
			addon.Definitions->Load(api);
			AddonDefs.insert({ aPath, addon });
		}
		else
		{
			LogWarning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Added to blacklist.", path, addon.Definitions->APIVersion);
			Blacklist.insert(aPath);
			FreeLibrary(hMod);
		}
	}
	void UnloadAddon(std::filesystem::path aPath)
	{
		std::string path = aPath.string();

		/* cache name for warning message and already release defs */
		std::string name = AddonDefs[aPath].Definitions->Name;
		int signature = AddonDefs[aPath].Definitions->Signature;

		AddonDefs[aPath].Definitions->Unload();

		if (AddonDefs[aPath].Module)
		{
			if (!FreeLibrary(AddonDefs[aPath].Module))
			{
				LogWarning(CH_LOADER, "Couldn't unload \"%s\". FreeLibrary() call failed. (%s)", name.c_str(), path.c_str());
				return;
			}
		}

		AddonDefs[aPath].Definitions = nullptr;

		/* Verify all APIs don't have any unreleased references to the addons address space */
		void* startAddress = AddonDefs[aPath].Module;
		void* endAddress = ((PBYTE)AddonDefs[aPath].Module) + AddonDefs[aPath].ModuleSize;

		int leftoverRefs = 0;
		leftoverRefs += Events::Verify(startAddress, endAddress);
		leftoverRefs += GUI::Verify(startAddress, endAddress);
		leftoverRefs += GUI::QuickAccess::Verify(startAddress, endAddress);
		leftoverRefs += Keybinds::Verify(startAddress, endAddress);
		leftoverRefs += LogHandler::Verify(startAddress, endAddress);

		if (leftoverRefs > 0)
		{
			LogWarning(CH_LOADER, "Removed %d unreleased references from \"%s\". Make sure your addon releases all references during Addon::Unload().", leftoverRefs, name.c_str());
		}

		AddonDefs[aPath].Module = nullptr;
		AddonDefs[aPath] = {};

		AddonDefs.erase(aPath);

		LogInfo(CH_LOADER, "Unloaded addon: %s", path.c_str());
	}
	void DetectAddonsLoop()
	{
		for (;;)
		{
			if (State::AddonHost == ENexusState::ADDONS_SHUTDOWN) { return; }

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
				
				Mutex.lock();
				{
					for (const auto& [path, addon] : AddonDefs)
					{
						if (onDiskLeft.find(path) != onDiskLeft.end())
						{
							/* addon is still present on disk, remove from the items that need to be checked */
							onDiskLeft.erase(path);
						}
						else
						{
							/* addon is no longer on disk -> unload afterwards, we can't modify this array */
							QueuedAddon q{};
							q.Action = ELoaderAction::Unload;
							q.Path = path;
							QueuedAddons.push_back(q);
						}
					}
				}
				Mutex.unlock();

				/* next, check if these files are on the blacklist */
				std::set <std::filesystem::path> unblacklist;

				Mutex.lock();
				{
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
						{
							QueuedAddon q{};
							q.Action = ELoaderAction::Load;
							q.Path = path;
							QueuedAddons.push_back(q);
						}
						Mutex.unlock();
					}
				}
			}

			PreviousFiles = onDisk;

			Sleep(5000);
		}
	}

	AddonAPI* GetAddonAPI(int aVersion)
	{
		// API defs with that version already exist, just return them
		if (ApiDefs.find(aVersion) != ApiDefs.end())
		{
			return ApiDefs[aVersion];
		}

		AddonAPI* api;

		// create the requested version, add it to the map and return it
		switch (aVersion)
		{
		case 1:
			api = new AddonAPI1();

			((AddonAPI1*)api)->SwapChain = Renderer::SwapChain;
			((AddonAPI1*)api)->ImguiContext = Renderer::GuiContext;
			((AddonAPI1*)api)->ImguiMalloc = ImGui::MemAlloc;
			((AddonAPI1*)api)->ImguiFree = ImGui::MemFree;
			((AddonAPI1*)api)->RegisterRender = GUI::Register;
			((AddonAPI1*)api)->UnregisterRender = GUI::Unregister;

			((AddonAPI1*)api)->CreateHook = MH_CreateHook;
			((AddonAPI1*)api)->RemoveHook = MH_RemoveHook;
			((AddonAPI1*)api)->EnableHook = MH_EnableHook;
			((AddonAPI1*)api)->DisableHook = MH_DisableHook;

			((AddonAPI1*)api)->Log = LogMessageAddon;
			((AddonAPI1*)api)->RegisterLogger = RegisterLogger;
			((AddonAPI1*)api)->UnregisterLogger = UnregisterLogger;

			((AddonAPI1*)api)->RaiseEvent = Events::Raise;
			((AddonAPI1*)api)->SubscribeEvent = Events::Subscribe;
			((AddonAPI1*)api)->UnsubscribeEvent = Events::Unsubscribe;

			((AddonAPI1*)api)->RegisterWndProc = WndProc::Register;
			((AddonAPI1*)api)->UnregisterWndProc = WndProc::Unregister;

			((AddonAPI1*)api)->RegisterKeybind = Keybinds::Register;
			((AddonAPI1*)api)->UnregisterKeybind = Keybinds::Unregister;

			((AddonAPI1*)api)->GetResource = DataLink::GetResource;
			((AddonAPI1*)api)->ShareResource = DataLink::ShareResource;

			((AddonAPI1*)api)->GetTexture = TextureLoader::Get;
			((AddonAPI1*)api)->LoadTextureFromFile = TextureLoader::LoadFromFile;
			((AddonAPI1*)api)->LoadTextureFromResource = TextureLoader::LoadFromResource;

			((AddonAPI1*)api)->AddShortcut = GUI::QuickAccess::AddShortcut;
			((AddonAPI1*)api)->RemoveShortcut = GUI::QuickAccess::RemoveShortcut;
			((AddonAPI1*)api)->AddSimpleShortcut = GUI::QuickAccess::AddSimpleShortcut;
			((AddonAPI1*)api)->RemoveSimpleShortcut = GUI::QuickAccess::RemoveSimpleShortcut;

			ApiDefs.insert({ aVersion, api });
			return api;
		}

		// there is no matching version
		return nullptr;
	}
}