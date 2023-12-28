#include "Loader.h"

#include <set>
#include <Windows.h>
#include <Psapi.h>

#include "core.h"
#include "State.h"
#include "Shared.h"
#include "Paths.h"
#include "Renderer.h"
#include "Consts.h"

#include "AddonDefinition.h"
#include "FuncDefs.h"

#include "Mumble/LinkedMem.h"
#include "Logging/LogHandler.h"
#include "Events/EventHandler.h"
#include "WndProc/WndProcHandler.h"
#include "Keybinds/KeybindHandler.h"
#include "imgui/imgui.h"
#include "minhook/mh_hook.h"
#include "DataLink/DataLink.h"
#include "Textures/TextureLoader.h"
#include "GUI/GUI.h"
#include "GUI/Widgets/QuickAccess/QuickAccess.h"
#include "Updater/Updater.h"

namespace Loader
{
	std::mutex Mutex;
	std::unordered_map<std::filesystem::path, ELoaderAction> QueuedAddons;
	std::unordered_map<std::filesystem::path, Addon*> Addons;
	std::map<int, AddonAPI*> ApiDefs;

	PIDLIST_ABSOLUTE FSItemList;
	ULONG FSNotifierID;

	std::string dll = ".dll";

	void Initialize()
	{
		if (State::Nexus == ENexusState::LOADED)
		{
			FSItemList = ILCreateFromPathA(Path::GetAddonDirectory(nullptr));
			if (FSItemList == 0) {
				LogCritical(CH_LOADER, "Loader disabled. Reason: ILCreateFromPathA(Path::D_GW2_ADDONS) returned 0.");
			}
			else
			{
				SHChangeNotifyEntry changeentry{};
				changeentry.pidl = FSItemList;
				changeentry.fRecursive = false;
				FSNotifierID = SHChangeNotifyRegister(
					Renderer::WindowHandle,
					SHCNRF_InterruptLevel | SHCNRF_NewDelivery,
					SHCNE_UPDATEITEM,
					WM_ADDONDIRUPDATE,
					1,
					&changeentry
				);

				if (FSNotifierID <= 0)
				{
					LogCritical(CH_LOADER, "Loader disabled. Reason: SHChangeNotifyRegister(...) returned 0.");
				}
				else
				{
					DetectAddonChanges(); // Invoke once because addon dir doesn't change on init
				}
			}
		}
	}
	void Shutdown()
	{
		if (State::Nexus == ENexusState::SHUTTING_DOWN)
		{
			if (FSNotifierID != 0)
			{
				SHChangeNotifyDeregister(FSNotifierID);
				FSNotifierID = 0;
			}

			if (FSItemList != 0)
			{
				ILFree(FSItemList);
				FSItemList = 0;
			}

			const std::lock_guard<std::mutex> lock(Mutex);
			{
				while (Addons.size() != 0)
				{
					UnloadAddon(Addons.begin()->first);
					if (Addons.begin()->second->Module)
					{
						Log(CH_LOADER, "Despite being unloaded \"%s\" still has an HMODULE defined, calling FreeLibrary().", Addons.begin()->first.string().c_str());
						FreeLibrary(Addons.begin()->second->Module);
					}
					Addons.erase(Addons.begin());
				}
			}
		}
	}

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_ADDONDIRUPDATE)
		{
			PIDLIST_ABSOLUTE* ppidl;
			LONG event;
			HANDLE notificationLock = SHChangeNotification_Lock((HANDLE)wParam, lParam, &ppidl, &event);

			if (notificationLock != 0)
			{
				if (event == SHCNE_UPDATEITEM)
				{
					char path[MAX_PATH];
					if (SHGetPathFromIDList(ppidl[0], path))
					{
						if (Path::D_GW2_ADDONS == std::string(path))
						{
							
						}
					}
				}

				SHChangeNotification_Unlock(notificationLock);
			}

			return 0;
		}

		return uMsg;
	}

	void ProcessQueue()
	{
		const std::lock_guard<std::mutex> lock(Mutex);
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
			
			if (!Addons[it->first] || Addons[it->first]->State != EAddonState::Reload)
			{
				QueuedAddons.erase(it);
			}
		}
	}
	void QueueAddon(ELoaderAction aAction, const std::filesystem::path& aPath)
	{
		QueuedAddons.insert({ aPath, aAction });
	}

	void DetectAddonChanges()
	{
		Log(CH_LOADER, "Addon directory changes detected.");

		std::set<std::filesystem::path> onDisk;

		/* iterate over each file on disk and check if it's currently tracked */
		for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
		{
			std::filesystem::path path = entry.path();
			if (!entry.is_directory() && path.extension() == dll)
			{
				onDisk.insert(path);

				const std::lock_guard<std::mutex> lock(Mutex);
				if (Addons.find(path) == Addons.end())
				{
					// this file does not exist yet in the tracked addons/files
					QueueAddon(ELoaderAction::Load, path);
				}
			}
		}

		std::vector<std::filesystem::path> rem;

		const std::lock_guard<std::mutex> lock(Mutex);
		for (const auto& [path, addon] : Addons)
		{
			if (std::find(onDisk.begin(), onDisk.end(), path) == onDisk.end())
			{
				// the addon no longer exists on disk, unload it
				if (addon && addon->State == EAddonState::Loaded)
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
	}

	void PerformUpdateSwap(const std::filesystem::path& aPath)
	{
		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + ".old";
		std::filesystem::path pathUpdate = aPath.string() + ".update";

		try
		{
			std::filesystem::rename(aPath, pathOld);
			std::filesystem::rename(pathUpdate, aPath);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Log(CH_LOADER, "%s", fErr.what());
			return;
		}

		LogInfo(CH_LOADER, "Successfully updated %s.", aPath.string().c_str());
	}
	void CheckForUpdates()
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (auto& [path, addon] : Addons)
			{
				if (!addon->Definitions) { continue; }

				if (Updater::CheckForUpdate(path, addon->Definitions))
				{
					bool wasLoaded = false;
					if (addon->State == EAddonState::Loaded)
					{
						wasLoaded = true;
						UnloadAddon(path);
					}

					PerformUpdateSwap(path);

					if (wasLoaded)
					{
						addon->State = EAddonState::Reload;
					}
				}
			}
		}
	}

	void LoadAddon(const std::filesystem::path& aPath)
	{
		std::string path = aPath.string();
		const char* strPath = path.c_str();

		Addon* addon;

		if (Addons.find(aPath) != Addons.end())
		{
			addon = Addons[aPath];

			switch (addon->State)
			{
			case EAddonState::Loaded:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Already loaded.", strPath);
				return;

			case EAddonState::NotLoadedDuplicate:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Another addon with the same signature already loaded.", strPath);
				return;

			case EAddonState::Incompatible:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Incompatible.", strPath);
				return;
			case EAddonState::IncompatibleAPI:
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". API that doesn't exist was requested.", strPath);
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
		addon->Module = LoadLibraryA(strPath);

		/* load lib failed */
		if (!addon->Module)
		{
			LogDebug(CH_LOADER, "Failed LoadLibrary on \"%s\". Incompatible. Last Error: %u", strPath, GetLastError());
			addon->State = EAddonState::Incompatible;
			return;
		}

		/* doesn't have GetAddonDef */
		if (FindFunction(addon->Module, &getAddonDef, "GetAddonDef") == false)
		{
			LogDebug(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Incompatible.", strPath);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			return;
		}
		
		/* why has god forsaken me */
		AddonDefinition* tmpDefs = getAddonDef();

		if (tmpDefs == nullptr)
		{
			LogDebug(CH_LOADER, "\"%s\" is Nexus-compatible but returned a nullptr. Incompatible I guess?", strPath);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			return;
		}

		if (addon->State != EAddonState::Reload && Updater::CheckForUpdate(aPath, tmpDefs))
		{
			addon->State = EAddonState::Reload;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;

			PerformUpdateSwap(aPath);

			return;
		}

		/* doesn't fulfill min reqs */
		if (!tmpDefs->HasMinimumRequirements())
		{
			LogWarning(CH_LOADER, "\"%s\" does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as the Load function. Incompatible.", strPath);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			return;
		}

		/* check if duplicate signature */
		for (auto& it : Addons)
		{
			// if defs defined && not the same path && signature the same though (another && it.second->Definitions in the mix because could still be null during load)
			if (it.first != aPath && it.second->Definitions && it.second->Definitions->Signature == tmpDefs->Signature && it.second->State == EAddonState::Loaded)
			{
				LogWarning(CH_LOADER, "\"%s\" or another addon with this signature (%d) is already loaded. Added to blacklist.", strPath, tmpDefs->Signature);
				addon->State = EAddonState::NotLoadedDuplicate;
				FreeLibrary(addon->Module);
				addon->Module = nullptr;
				return;
			}
		}

		MODULEINFO moduleInfo{};
		GetModuleInformation(GetCurrentProcess(), addon->Module, &moduleInfo, sizeof(moduleInfo));

		AddonAPI* api = GetAddonAPI(tmpDefs->APIVersion); // will be nullptr if doesn't exist or APIVersion = 0

		// free the old stuff
		if (addon->Definitions != nullptr)
		{
			// is this necessary?
			free((char*)addon->Definitions->Name);
			free((char*)addon->Definitions->Author);
			free((char*)addon->Definitions->Description);
			if (addon->Definitions->UpdateLink)
			{
				free((char*)addon->Definitions->UpdateLink);
			}
			delete addon->Definitions;
		}
		
		// Allocate new memory and copy data
		addon->Definitions = new AddonDefinition(*tmpDefs);

		// Allocate and copy strings, considering possible null pointers
		addon->Definitions->Name = _strdup(tmpDefs->Name);
		addon->Definitions->Author = _strdup(tmpDefs->Author);
		addon->Definitions->Description = _strdup(tmpDefs->Description);
		addon->Definitions->UpdateLink = (tmpDefs->UpdateLink) ? _strdup(tmpDefs->UpdateLink) : nullptr;

		// if no addon api was requested or if the requested addon api exists
		// else invalid addon, don't load
		if (addon->Definitions->APIVersion == 0 || api != nullptr)
		{
			addon->ModuleSize = moduleInfo.SizeOfImage;

			if (addon->Definitions->APIVersion == 0)
			{
				LogInfo(CH_LOADER, "Loaded addon: %s (Signature %d) [%p - %p] (No API was requested.)", strPath, addon->Definitions->Signature, addon->Module, ((PBYTE)addon->Module) + moduleInfo.SizeOfImage);
			}
			else
			{
				LogInfo(CH_LOADER, "Loaded addon: %s (Signature %d) [%p - %p] (API Version %d was requested.)", strPath, addon->Definitions->Signature, addon->Module, ((PBYTE)addon->Module) + moduleInfo.SizeOfImage, addon->Definitions->APIVersion);
			}
			addon->Definitions->Load(api);
			addon->State = EAddonState::Loaded;
		}
		else
		{
			LogWarning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Incompatible.", strPath, addon->Definitions->APIVersion);
			addon->State = EAddonState::IncompatibleAPI;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
		}
	}
	void UnloadAddon(const std::filesystem::path& aPath)
	{
		std::string path = aPath.string();
		const char* strPath = path.c_str();

		Addon* addon;

		if (Addons.find(aPath) != Addons.end())
		{
			addon = Addons[aPath];

			if (!addon->Module)
			{
				switch (addon->State)
				{
				case EAddonState::NotLoaded:
					LogWarning(CH_LOADER, "Cancelled unload of \"%s\". Already unloaded.", strPath);
					return;
				case EAddonState::NotLoadedDuplicate:
					LogWarning(CH_LOADER, "Cancelled unload of \"%s\". EAddonState::NotLoadedDuplicate.", strPath);
					return;

				case EAddonState::Incompatible:
					LogWarning(CH_LOADER, "Cancelled unload of \"%s\". EAddonState::Incompatible.", strPath);
					return;
				case EAddonState::IncompatibleAPI:
					LogWarning(CH_LOADER, "Cancelled unload of \"%s\". EAddonState::IncompatibleAPI.", strPath);
					return;
				}
			}
		}
		else
		{
			return;
		}

		if (addon->Definitions)
		{
			if (!addon->Definitions->Unload || addon->Definitions->HasFlag(EAddonFlags::DisableHotloading))
			{
				addon->Module = 0;
				LogWarning(CH_LOADER, "Prevented unloading \"%s\" because either no Unload function is defined or Hotloading is explicitly disabled. (%s)", addon->Definitions->Name, strPath);
				return;
			}

			addon->Definitions->Unload();
		}

		if (addon->Module)
		{
			if (addon->ModuleSize > 0)
			{
				/* Verify all APIs don't have any unreleased references to the addons address space */
				void* startAddress = addon->Module;
				void* endAddress = ((PBYTE)addon->Module) + addon->ModuleSize;

				int leftoverRefs = 0;
				leftoverRefs += Events::Verify(startAddress, endAddress);
				leftoverRefs += GUI::Verify(startAddress, endAddress);
				leftoverRefs += GUI::QuickAccess::Verify(startAddress, endAddress);
				leftoverRefs += Keybinds::Verify(startAddress, endAddress);
				leftoverRefs += WndProc::Verify(startAddress, endAddress);

				if (leftoverRefs > 0)
				{
					LogWarning(CH_LOADER, "Removed %d unreleased references from \"%s\". Make sure your addon releases all references during Addon::Unload().", leftoverRefs, strPath);
				}
			}

			int freeCalls = 0;

			while (FreeLibrary(addon->Module))
			{
				freeCalls++;
			}

			if (freeCalls == 0)
			{
				LogWarning(CH_LOADER, "Couldn't unload \"%s\". FreeLibrary() call failed.", strPath);
				return;
			}
			else
			{
				LogDebug(CH_LOADER, "Called FreeLibrary() %d times on \"%s\".", freeCalls, strPath);
			}
		}

		addon->Module = nullptr;
		addon->ModuleSize = 0;

		addon->State = EAddonState::NotLoaded;

		if (!std::filesystem::exists(aPath))
		{
			Addons.erase(aPath);
		}

		LogInfo(CH_LOADER, "Unloaded addon: %s", strPath);
	}
	void UninstallAddon(const std::filesystem::path& aPath)
	{
		// if file exists, delete it
		if (std::filesystem::exists(aPath))
		{
			UnloadAddon(aPath);
			std::filesystem::remove(aPath.string().c_str());
			Addons.erase(aPath);
			LogInfo(CH_LOADER, "Uninstalled addon: %s", aPath.string().c_str());
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

		case 2:
			api = new AddonAPI2();

			((AddonAPI2*)api)->SwapChain = Renderer::SwapChain;
			((AddonAPI2*)api)->ImguiContext = Renderer::GuiContext;
			((AddonAPI2*)api)->ImguiMalloc = ImGui::MemAlloc;
			((AddonAPI2*)api)->ImguiFree = ImGui::MemFree;
			((AddonAPI2*)api)->RegisterRender = GUI::Register;
			((AddonAPI2*)api)->UnregisterRender = GUI::Unregister;

			((AddonAPI2*)api)->GetGameDirectory = Path::GetGameDirectory;
			((AddonAPI2*)api)->GetAddonDirectory = Path::GetAddonDirectory;
			((AddonAPI2*)api)->GetCommonDirectory = Path::GetCommonDirectory;

			((AddonAPI2*)api)->CreateHook = MH_CreateHook;
			((AddonAPI2*)api)->RemoveHook = MH_RemoveHook;
			((AddonAPI2*)api)->EnableHook = MH_EnableHook;
			((AddonAPI2*)api)->DisableHook = MH_DisableHook;

			((AddonAPI2*)api)->Log = LogMessageAddon;

			((AddonAPI2*)api)->RaiseEvent = Events::Raise;
			((AddonAPI2*)api)->SubscribeEvent = Events::Subscribe;
			((AddonAPI2*)api)->UnsubscribeEvent = Events::Unsubscribe;

			((AddonAPI2*)api)->RegisterWndProc = WndProc::Register;
			((AddonAPI2*)api)->UnregisterWndProc = WndProc::Unregister;

			((AddonAPI2*)api)->RegisterKeybindWithString = Keybinds::Register;
			((AddonAPI2*)api)->RegisterKeybindWithStruct = Keybinds::RegisterWithStruct;
			((AddonAPI2*)api)->UnregisterKeybind = Keybinds::Unregister;

			((AddonAPI2*)api)->GetResource = DataLink::GetResource;
			((AddonAPI2*)api)->ShareResource = DataLink::ShareResource;

			((AddonAPI2*)api)->GetTexture = TextureLoader::Get;
			((AddonAPI2*)api)->LoadTextureFromFile = TextureLoader::LoadFromFile;
			((AddonAPI2*)api)->LoadTextureFromResource = TextureLoader::LoadFromResource;
			((AddonAPI2*)api)->LoadTextureFromURL = TextureLoader::LoadFromURL;
			((AddonAPI2*)api)->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

			((AddonAPI2*)api)->AddShortcut = GUI::QuickAccess::AddShortcut;
			((AddonAPI2*)api)->RemoveShortcut = GUI::QuickAccess::RemoveShortcut;
			((AddonAPI2*)api)->NotifyShortcut = GUI::QuickAccess::NotifyShortcut;
			((AddonAPI2*)api)->AddSimpleShortcut = GUI::QuickAccess::AddSimpleShortcut;
			((AddonAPI2*)api)->RemoveSimpleShortcut = GUI::QuickAccess::RemoveSimpleShortcut;

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
		case 2:
			return sizeof(AddonAPI2);
		}

		return 0;
	}
}