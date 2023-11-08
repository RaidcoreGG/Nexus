#include "Loader.h"

namespace Loader
{
	std::mutex Mutex;
	std::map<std::filesystem::path, ELoaderAction> QueuedAddons;
	std::map<std::filesystem::path, Addon*> Addons;
	std::map<int, AddonAPI*> ApiDefs;

	std::thread LoaderThread;

	std::string dll = ".dll";

	void Initialize()
	{
		if (State::Nexus < ENexusState::LOADED)
		{
			LoaderThread = std::thread(DetectAddonsLoop);
			LoaderThread.detach();
		}
	}
	void Shutdown()
	{
		if (State::Nexus < ENexusState::SHUTDOWN)
		{
			Loader::Mutex.lock();
			{
				while (Addons.size() != 0)
				{
					UnloadAddon(Addons.begin()->first);
				}
			}
			Loader::Mutex.unlock();
		}
	}

	void ProcessQueue()
	{
		Loader::Mutex.lock();
		while (QueuedAddons.size() > 0)
		{
			auto it = QueuedAddons.begin();

			switch (it->second)
			{
			case ELoaderAction::Load:
				LoadAddon(it->first);
				break;
			case ELoaderAction::Unload:
				UnloadAddon(it->first);
				break;
			case ELoaderAction::Uninstall:
				UninstallAddon(it->first);
				break;
			}

			QueuedAddons.erase(it);
		}
		Loader::Mutex.unlock();
	}
	void QueueAddon(ELoaderAction aAction, std::filesystem::path aPath)
	{
		QueuedAddons.insert({ aPath, aAction });
	}

	void LoadAddon(std::filesystem::path aPath)
	{
		std::string pathStr = aPath.string();
		const char* path = pathStr.c_str();

		Addon* addon;

		if (Addons.find(aPath) != Addons.end())
		{
			addon = Addons[aPath];

			switch (addon->State)
			{
			case EAddonState::Loaded:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Already loaded.", path);
				return;

			case EAddonState::NotLoaded:
				LogWarning(CH_LOADER, "Reloading \"%s\" from an explicitly unloaded state.", path);
				break;
			case EAddonState::NotLoadedDuplicate:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Another addon with the same signature already loaded.", path);
				return;

			case EAddonState::Incompatible:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Incompatible.", path);
				return;
			case EAddonState::IncompatibleAPI:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". API that doesn't exist was requested.", path);
				return;
			}
		}
		else
		{
			addon = new Addon{};
			addon->State = EAddonState::None;
			Addons.insert({ aPath, addon });
		}

		GETADDONDEF getAddonDef = 0;
		HMODULE hMod = LoadLibraryA(path);

		/* load lib failed */
		if (!hMod)
		{
			LogDebug(CH_LOADER, "Failed LoadLibrary on \"%s\". Incompatible.", path);
			addon->State = EAddonState::Incompatible;
			return;
		}

		/* doesn't have GetAddonDef */
		if (FindFunction(hMod, &getAddonDef, "GetAddonDef") == false)
		{
			LogDebug(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Incompatible.", path);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(hMod);
			return;
		}
		
		/* why has god forsaken me */
		AddonDefinition* tmpDefs = getAddonDef();
		AddonDefinition* defs = new AddonDefinition();
		memcpy(defs, tmpDefs, sizeof(AddonDefinition));
		defs->Name = new char[strlen(tmpDefs->Name) + 1];
		strcpy((char*)defs->Name, tmpDefs->Name);
		defs->Author = new char[strlen(tmpDefs->Author) + 1];
		strcpy((char*)defs->Author, tmpDefs->Author);
		defs->Description = new char[strlen(tmpDefs->Description) + 1];
		strcpy((char*)defs->Description, tmpDefs->Description);
		defs->UpdateLink = new char[strlen(tmpDefs->UpdateLink) + 1];
		strcpy((char*)defs->UpdateLink, tmpDefs->UpdateLink);

		/* doesn't full fill min reqs */
		if (hMod && !defs->HasMinimumRequirements())
		{
			LogWarning(CH_LOADER, "\"%s\" does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as Load and Unload functions. Incompatible.", path);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(hMod);
			return;
		}

		/* check if duplicate signature */
		for (auto& it : Addons)
		{
			// if defs defined && not the same path && signature the same though
			if (it.second->Definitions != nullptr && it.first != aPath && it.second->Definitions->Signature == defs->Signature)
			{
				LogWarning(CH_LOADER, "\"%s\" or another addon with sig%d is already loaded. Added to blacklist.", path, defs->Signature);
				addon->State = EAddonState::NotLoadedDuplicate;
				FreeLibrary(hMod);
				return;
			}
		}

		MODULEINFO moduleInfo;
		GetModuleInformation(GetCurrentProcess(), hMod, &moduleInfo, sizeof(moduleInfo));

		AddonAPI* api = GetAddonAPI(defs->APIVersion); // will be nullptr if doesn't exist or APIVersion = 0

		// if no addon api was requested or if the requested addon api exists
		// else invalid addon, don't load
		if (defs->APIVersion == 0 || api != nullptr)
		{
			addon->Module = hMod;
			addon->ModuleSize = moduleInfo.SizeOfImage;

			// free the old stuff
			if (addon->Definitions != nullptr)
			{
				// is this necessary?
				delete[] addon->Definitions->Name;
				delete[] addon->Definitions->Author;
				delete[] addon->Definitions->Description;
				delete[] addon->Definitions->UpdateLink;
				delete addon->Definitions;
			}

			addon->Definitions = defs;

			if (defs->APIVersion == 0)
			{
				LogInfo(CH_LOADER, "Loaded addon: %s [%p - %p] (No API was requested.)", path, hMod, ((PBYTE)hMod) + moduleInfo.SizeOfImage);
			}
			else
			{
				LogInfo(CH_LOADER, "Loaded addon: %s [%p - %p] (API Version %d was requested.)", path, hMod, ((PBYTE)hMod) + moduleInfo.SizeOfImage, defs->APIVersion);
			}
			addon->Definitions->Load(api);
			addon->State = EAddonState::Loaded;
		}
		else
		{
			LogWarning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Added to blacklist.", path, defs->APIVersion);
			addon->State = EAddonState::IncompatibleAPI;
			FreeLibrary(hMod);
		}
	}
	void UnloadAddon(std::filesystem::path aPath)
	{
		std::string pathStr = aPath.string();
		const char* path = pathStr.c_str();

		Addon* addon;

		if (Addons.find(aPath) != Addons.end())
		{
			addon = Addons[aPath];

			switch (addon->State)
			{
			case EAddonState::NotLoaded:
				LogWarning(CH_LOADER, "Cancelled unloading \"%s\". Already unloaded.", path);
				return;
			case EAddonState::NotLoadedDuplicate:
				LogWarning(CH_LOADER, "Cancelled unloading \"%s\". EAddonState::NotLoadedDuplicate. This should never happen.", path);
				return;

			case EAddonState::Incompatible:
				LogWarning(CH_LOADER, "Cancelled unloading \"%s\". EAddonState::Incompatible. This should never happen.", path);
				return;
			case EAddonState::IncompatibleAPI:
				LogWarning(CH_LOADER, "Cancelled unloading \"%s\". EAddonState::IncompatibleAPI. This should never happen.", path);
				return;
			}
		}
		else
		{
			addon = new Addon{};
			addon->State = EAddonState::None;
			Addons.insert({ aPath, addon });
		}

		if (Addons[aPath]->Definitions != nullptr)
		{
			/* cache name for warning message and already release defs */
			std::string name = Addons[aPath]->Definitions->Name;

			Addons[aPath]->Definitions->Unload();

			if (Addons[aPath]->Module)
			{
				if (!FreeLibrary(Addons[aPath]->Module))
				{
					LogWarning(CH_LOADER, "Couldn't unload \"%s\". FreeLibrary() call failed. (%s)", name.c_str(), path);
					return;
				}
			}
		}
		else
		{
			LogCritical(CH_LOADER, "Fatal Error. \"%s\" : Definitions == nullptr", path);
		}

		if (Addons[aPath]->Module != nullptr && Addons[aPath]->ModuleSize > 0)
		{
			/* Verify all APIs don't have any unreleased references to the addons address space */
			void* startAddress = Addons[aPath]->Module;
			void* endAddress = ((PBYTE)Addons[aPath]->Module) + Addons[aPath]->ModuleSize;

			int leftoverRefs = 0;
			leftoverRefs += Events::Verify(startAddress, endAddress);
			leftoverRefs += GUI::Verify(startAddress, endAddress);
			leftoverRefs += GUI::QuickAccess::Verify(startAddress, endAddress);
			leftoverRefs += Keybinds::Verify(startAddress, endAddress);
			leftoverRefs += LogHandler::Verify(startAddress, endAddress);
			leftoverRefs += WndProc::Verify(startAddress, endAddress);

			if (leftoverRefs > 0)
			{
				LogWarning(CH_LOADER, "Removed %d unreleased references from \"%s\". Make sure your addon releases all references during Addon::Unload().", leftoverRefs, path);
			}

			Addons[aPath]->Module = nullptr;
			Addons[aPath]->ModuleSize = 0;
		}
		else
		{
			LogCritical(CH_LOADER, "Fatal Error. \"%s\" : Addons[aPath].Module == nullptr || Addons[aPath].ModuleSize <= 0", path);
		}
		
		Addons[aPath]->State = EAddonState::NotLoaded;

		if (!std::filesystem::exists(aPath))
		{
			Addons.erase(aPath);
		}

		LogInfo(CH_LOADER, "Unloaded addon: %s", path);
	}
	void UninstallAddon(std::filesystem::path aPath)
	{
		std::string path = aPath.string();

		// if file exists, delete it
		if (std::filesystem::exists(aPath))
		{
			UnloadAddon(aPath);
			std::remove(aPath.string().c_str());
			Addons.erase(aPath);
			LogInfo(CH_LOADER, "Uninstalled addon: %s", path.c_str());
		}
	}
	void DetectAddonsLoop()
	{
		for (;;)
		{
			if (State::Nexus >= ENexusState::SHUTDOWN) { return; }

			if (State::Nexus == ENexusState::READY)
			{
				std::set<std::filesystem::path> onDisk;

				/* iterate over each file on disk and check if it's currently tracked */
				for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
				{
					std::filesystem::path path = entry.path();
					if (!entry.is_directory() && path.extension() == dll)
					{
						onDisk.insert(path);

						Loader::Mutex.lock();
						if (Addons.find(path) == Addons.end())
						{
							// this file does not exist yet in the tracked addons/files
							QueueAddon(ELoaderAction::Load, path);
						}
						Loader::Mutex.unlock();
					}
				}

				std::vector<std::filesystem::path> rem;

				Loader::Mutex.lock();
				for (const auto& [path, addon] : Addons)
				{
					if (std::find(onDisk.begin(), onDisk.end(), path) == onDisk.end())
					{
						// the addon no longer exists on disk, unload it
						if (addon->State == EAddonState::Loaded)
						{
							QueueAddon(ELoaderAction::Unload, path);
						}
						else
						{
							// sanity fallback
							rem.push_back(path);
						}
					}
				}
				for (std::filesystem::path p : rem)
				{
					Addons.erase(p);
				}
				Loader::Mutex.unlock();

				//ProcessQueue();
			}
			
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

			((AddonAPI1*)api)->GetGameDirectory	= Path::GetGameDirectory;
			((AddonAPI1*)api)->GetAddonDirectory = Path::GetAddonDirectory;
			((AddonAPI1*)api)->GetCommonDirectory = Path::GetCommonDirectory;

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

			((AddonAPI1*)api)->RegisterKeybindWithString = Keybinds::Register;
			((AddonAPI1*)api)->RegisterKeybindWithStruct = Keybinds::RegisterWithStruct;
			((AddonAPI1*)api)->UnregisterKeybind = Keybinds::Unregister;

			((AddonAPI1*)api)->GetResource = DataLink::GetResource;
			((AddonAPI1*)api)->ShareResource = DataLink::ShareResource;

			((AddonAPI1*)api)->GetTexture = TextureLoader::Get;
			((AddonAPI1*)api)->LoadTextureFromFile = TextureLoader::LoadFromFile;
			((AddonAPI1*)api)->LoadTextureFromResource = TextureLoader::LoadFromResource;
			((AddonAPI1*)api)->LoadTextureFromURL = TextureLoader::LoadFromURL;

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
	long GetAddonAPISize(int aVersion)
	{
		switch (aVersion)
		{
		case 1:
			return sizeof(AddonAPI1);
		}

		return 0;
	}
}