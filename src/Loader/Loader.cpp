#include "Loader.h"

#include <Windows.h>
#include <Psapi.h>
#include <regex>
#include <malloc.h>
#include <vector>
#include <fstream>

#include "resource.h"

#include "core.h"
#include "State.h"
#include "Shared.h"
#include "Paths.h"
#include "Renderer.h"
#include "Consts.h"

#include "AddonDefinition.h"
#include "FuncDefs.h"

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

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "httplib/httplib.h"

#define LOADER_WAITTIME_MS 1

namespace Loader
{
	std::mutex					Mutex;
	std::vector<LibraryAddon*>	AddonLibrary;
	std::unordered_map<
		std::filesystem::path,
		ELoaderAction
	>							QueuedAddons;
	std::map<
		signed int,
		StoredAddon
	>							AddonConfig;
	std::map<
		std::filesystem::path,
		Addon*
	>							Addons;
	std::map<int, AddonAPI*>	ApiDefs;

	int							DirectoryChangeCountdown = 0;
	std::condition_variable		ConVar;
	std::mutex					ThreadMutex;
	std::thread					LoaderThread;
	bool						IsSuspended = false;

	PIDLIST_ABSOLUTE			FSItemList;
	ULONG						FSNotifierID;

	std::string extDll			= ".dll";
	std::string extUpdate		= ".update";
	std::string extOld			= ".old";
	std::string extUninstall	= ".uninstall";

	void Initialize()
	{
		if (State::Nexus == ENexusState::LOADED)
		{
			if (std::filesystem::exists(Path::F_ADDONCONFIG))
			{
				{
					try
					{
						std::ifstream file(Path::F_ADDONCONFIG);

						json cfg = json::parse(file);
						for (json addonInfo : cfg)
						{
							signed int signature = 0;
							StoredAddon storedAddonInfo{};
							if (!addonInfo["Signature"].is_null()) { addonInfo["Signature"].get_to(signature); }
							if (!addonInfo["IsPausingUpdates"].is_null()) { addonInfo["IsPausingUpdates"].get_to(storedAddonInfo.IsPausingUpdates); }
							if (!addonInfo["IsLoaded"].is_null()) { addonInfo["IsLoaded"].get_to(storedAddonInfo.IsLoaded); }
							if (!addonInfo["IsDisabledUntilUpdate"].is_null()) { addonInfo["IsDisabledUntilUpdate"].get_to(storedAddonInfo.IsDisabledUntilUpdate); }

							if (signature == 0) { continue; }

							AddonConfig[signature] = storedAddonInfo;
						}

						file.close();
					}
					catch (json::parse_error& ex)
					{
						LogWarning(CH_KEYBINDS, "AddonConfig.json could not be parsed. Error: %s", ex.what());
					}
				}
			}

			FSItemList = ILCreateFromPathA(Path::GetAddonDirectory(nullptr));
			if (FSItemList == 0) {
				LogCritical(CH_LOADER, "Loader disabled. Reason: ILCreateFromPathA(Path::D_GW2_ADDONS) returned 0.");
				return;
			}

			SHChangeNotifyEntry changeentry{};
			changeentry.pidl = FSItemList;
			changeentry.fRecursive = false;
			FSNotifierID = SHChangeNotifyRegister(
				Renderer::WindowHandle,
				SHCNRF_InterruptLevel | SHCNRF_NewDelivery,
				SHCNE_UPDATEITEM | SHCNE_UPDATEDIR,
				WM_ADDONDIRUPDATE,
				1,
				&changeentry
			);

			if (FSNotifierID <= 0)
			{
				LogCritical(CH_LOADER, "Loader disabled. Reason: SHChangeNotifyRegister(...) returned 0.");
				return;
			}

			json response = RaidcoreAPI->Get("/addonlibrary");

			if (!response.is_null())
			{
				for (const auto& addon : response)
				{
					LibraryAddon* newAddon = new LibraryAddon{};
					newAddon->Signature = addon["id"];
					newAddon->Name = addon["name"];
					newAddon->Description = addon["description"];
					newAddon->Provider = GetProvider(addon["download"]);
					newAddon->DownloadURL = addon["download"];
					AddonLibrary.push_back(newAddon);
				}

				std::sort(AddonLibrary.begin(), AddonLibrary.end(), [](LibraryAddon* a, LibraryAddon* b) {
					return a->Name < b->Name;
					});
			}
			else
			{
				LogWarning(CH_CORE, "Error parsing API response for /addonlibrary.");
			}

			LoaderThread = std::thread(ProcessChanges);
			LoaderThread.detach();
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
				json cfg = json::array();

				for (auto it : Addons)
				{
					Addon* addon = it.second;

					if (!addon->Definitions) { continue; }
					if (addon->Definitions->Signature == -19392669) { continue; } // skip bridge

					json addonInfo =
					{
						{"Signature", addon->Definitions->Signature},
						{"IsLoaded", addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED ? true : false},
					};

					auto cfgIt = AddonConfig.find(addon->Definitions->Signature);
					if (cfgIt != AddonConfig.end())
					{
						addonInfo["IsPausingUpdates"] = cfgIt->second.IsPausingUpdates;
						addonInfo["IsDisabledUntilUpdate"] = cfgIt->second.IsDisabledUntilUpdate;
					}

					cfg.push_back(addonInfo);
				}

				std::ofstream file(Path::F_ADDONCONFIG);
				file << cfg.dump(1, '\t') << std::endl;
				file.close();
			}

			{
				while (Addons.size() != 0)
				{
					UnloadAddon(Addons.begin()->first, true);
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
				if (event == SHCNE_UPDATEITEM || event == SHCNE_UPDATEDIR)
				{
					char path[MAX_PATH];
					if (SHGetPathFromIDList(ppidl[0], path))
					{
						if (Path::D_GW2_ADDONS == std::string(path))
						{
							NotifyChanges();
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
			case ELoaderAction::Reload:
				ReloadAddon(it->first);
				break;
			}
			
			/*
				if the action is not reload, then remove it after it was performed
				else check if the addon exists and if it does check, if it is anything but NotLoaded
				if it is, it was processed
			*/
			if (it->second != ELoaderAction::Reload)
			{
				QueuedAddons.erase(it);
			}
			else
			{
				auto addon = Addons.find(it->first);

				if (addon != Addons.end())
				{
					if (addon->second->State != EAddonState::NotLoaded)
					{
						QueuedAddons.erase(it);
					}
				}
			}
		}
	}
	void QueueAddon(ELoaderAction aAction, const std::filesystem::path& aPath)
	{
		auto it = QueuedAddons.find(aPath);

		if (it != QueuedAddons.end())
		{
			// already exists
			it->second = aAction;
		}
		else
		{
			// insert
			QueuedAddons.insert({ aPath, aAction });
		}
	}

	void NotifyChanges()
	{
		DirectoryChangeCountdown = LOADER_WAITTIME_MS;
		IsSuspended = false;
		ConVar.notify_all();
	}
	void ProcessChanges()
	{
		for (;;)
		{
			{
				std::unique_lock<std::mutex> lockThread(ThreadMutex);
				ConVar.wait(lockThread, [] { return !IsSuspended; });

				auto start_time = std::chrono::high_resolution_clock::now();
				while (DirectoryChangeCountdown > 0)
				{
					Sleep(1);
					DirectoryChangeCountdown -= 1;
				}
				auto end_time = std::chrono::high_resolution_clock::now();
				auto time = end_time - start_time;
				Log(CH_LOADER, "Processing changes after waiting for %ums.", time / std::chrono::milliseconds(1));
			}

			{
				const std::lock_guard<std::mutex> lock(Mutex);

				// check all tracked addons
				for (auto& it : Addons)
				{
					// if addon no longer on disk
					if (!std::filesystem::exists(it.first))
					{
						QueueAddon(ELoaderAction::Unload, it.first);
						continue;
					}

					// get md5 of each file currently on disk and compare to tracked md5
					// also check if an update is available (e.g. "addon.dll" + ".update" -> "addon.dll.update" exists)
					std::vector<unsigned char> md5 = MD5FromFile(it.first);
					std::filesystem::path updatePath = it.first.string() + extUpdate;
					if ((it.second->MD5.empty() || it.second->MD5 != md5) || std::filesystem::exists(updatePath))
					{
						QueueAddon(ELoaderAction::Reload, it.first);
					}
				}

				// check all other files in the directory
				for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
				{
					std::filesystem::path path = entry.path();

					// if (not a file || already tracked) -> skip
					if (std::filesystem::is_directory(path) || Addons.find(path) != Addons.end())
					{
						continue;
					}

					if (std::filesystem::file_size(path) == 0)
					{
						continue;
					}

					if (path.extension() == extDll)
					{
						QueueAddon(ELoaderAction::Load, path);
					}

					if (path.extension() == extUninstall)
					{
						try
						{
							std::filesystem::remove(path);
						}
						catch (std::filesystem::filesystem_error fErr)
						{
							LogDebug(CH_LOADER, "%s", fErr.what());
							return;
						}
					}
				}
			}
			
			IsSuspended = true;
		}
	}
	
	void LoadAddon(const std::filesystem::path& aPath, bool aIsReload)
	{
		std::string path = aPath.string();
		const char* strPath = path.c_str();

		bool firstLoad;

		Addon* addon;

		auto it = Addons.find(aPath);
		
		if (it != Addons.end())
		{
			firstLoad = false;
			addon = it->second;

			if (addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED)
			{
				//LogWarning(CH_LOADER, "Cancelled loading \"%s\". Already loaded.", strPath);
				return;
			}
		}
		else
		{
			firstLoad = true;
			addon = new Addon{};
			addon->State = EAddonState::None;
			Addons.insert({ aPath, addon });
		}

		UpdateSwapAddon(aPath);

		GETADDONDEF getAddonDef = 0;
		addon->MD5 = MD5FromFile(aPath);
		addon->Module = LoadLibraryA(strPath);

		/* load lib failed */
		if (!addon->Module)
		{
			LogWarning(CH_LOADER, "Failed LoadLibrary on \"%s\". Incompatible. Last Error: %u", strPath, GetLastError());
			addon->State = EAddonState::NotLoadedIncompatible;
			return;
		}

		/* doesn't have GetAddonDef */
		if (FindFunction(addon->Module, &getAddonDef, "GetAddonDef") == false)
		{
			LogWarning(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Incompatible.", strPath);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}
		
		AddonDefinition* tmpDefs = getAddonDef();

		if (tmpDefs == nullptr)
		{
			LogWarning(CH_LOADER, "\"%s\" is Nexus-compatible but returned a nullptr. Incompatible.", strPath);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}

		StoredAddon* addonInfo = nullptr;
		auto cfgIt = AddonConfig.find(tmpDefs->Signature);

		if (cfgIt != AddonConfig.end())
		{
			addonInfo = &cfgIt->second;
		}

		/* check if on load list && first load and also make sure this isn't the arc integration */
		if (!aIsReload && firstLoad && tmpDefs->Signature != -19392669)
		{
			// reload is thrown in the mix here because not only does a reload imply not first load
			// but also this is necessary to skip this segment for arcdps integration
			bool wasRequested = false;
			if (RequestedAddons.size() > 0)
			{
				for (signed int id : RequestedAddons)
				{
					if (id == tmpDefs->Signature)
					{
						wasRequested = true;
						break;
					}
				}
			}
			else if (addonInfo != nullptr && addonInfo->IsLoaded)
			{
				wasRequested = true;
			}

			if (!wasRequested)
			{
				LogInfo(CH_LOADER, "\"%s\" was not requested via start parameter and/or last state was disabled. Skipped.", strPath, tmpDefs->Signature);

				// free old and clone new anyway so the addon can be listed
				FreeAddonDefs(&addon->Definitions);
				CopyAddonDefs(tmpDefs, &addon->Definitions);

				FreeLibrary(addon->Module);
				addon->State = EAddonState::NotLoaded;
				addon->Module = nullptr;
				return;
			}
		}

		// if not already being reloaded check for update
		// also either addons must be enabled or there's no info on that yet
		if (!aIsReload && (addonInfo == nullptr || !addonInfo->IsPausingUpdates))
		{
			std::filesystem::path tmpPath = aPath.string();
			signed int tmpSig = tmpDefs->Signature;
			std::string tmpName = tmpDefs->Name;
			AddonVersion tmpVers = tmpDefs->Version;
			bool tmpLocked = tmpDefs->Unload == nullptr || tmpDefs->HasFlag(EAddonFlags::DisableHotloading);
			EUpdateProvider tmpProv = tmpDefs->Provider;
			std::string tmpLink = tmpDefs->UpdateLink != nullptr ? tmpDefs->UpdateLink : "";

			std::thread([tmpPath, tmpSig, tmpName, tmpVers, tmpLocked, tmpProv, tmpLink]()
				{
					if (UpdateAddon(tmpPath, tmpSig, tmpName, tmpVers, tmpProv, tmpLink))
					{
						LogInfo(CH_LOADER, "Update available for \"%s\".", tmpPath.string().c_str());
						QueueAddon(ELoaderAction::Reload, tmpPath);
					}
					else if (tmpLocked)
					{
						QueueAddon(ELoaderAction::Reload, tmpPath);
					}
				})
				.detach();

			if (tmpDefs->Unload == nullptr || tmpDefs->HasFlag(EAddonFlags::DisableHotloading))
			{
				FreeLibrary(addon->Module);
				addon->State = EAddonState::NotLoaded;
				addon->Module = nullptr;
				return;
			}
		}

		/* doesn't fulfill min reqs */
		if (!tmpDefs->HasMinimumRequirements())
		{
			LogWarning(CH_LOADER, "\"%s\" does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as the Load function. Incompatible.", strPath);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}

		/* check if duplicate signature */
		for (auto& it : Addons)
		{
			// if defs defined && not the same path && signature the same though (another && it.second->Definitions in the mix because could still be null during load)
			if (it.first != aPath && 
				it.second->Definitions && 
				it.second->Definitions->Signature == tmpDefs->Signature && 
				(it.second->State == EAddonState::Loaded || it.second->State == EAddonState::LoadedLOCKED))
			{
				LogWarning(CH_LOADER, "\"%s\" or another addon with this signature (%d) is already loaded. Added to blacklist.", strPath, tmpDefs->Signature);
				FreeLibrary(addon->Module);
				addon->State = EAddonState::NotLoadedDuplicate;
				addon->Module = nullptr;
				return;
			}
		}

		AddonAPI* api = GetAddonAPI(tmpDefs->APIVersion); // will be nullptr if doesn't exist or APIVersion = 0

		// free old and clone new
		FreeAddonDefs(&addon->Definitions);
		CopyAddonDefs(tmpDefs, &addon->Definitions);

		// if the api doesn't exist and there was one requested
		if (api == nullptr && addon->Definitions->APIVersion != 0)
		{
			LogWarning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Incompatible.", strPath, addon->Definitions->APIVersion);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatibleAPI;
			addon->Module = nullptr;
			return;
		}

		MODULEINFO moduleInfo{};
		GetModuleInformation(GetCurrentProcess(), addon->Module, &moduleInfo, sizeof(moduleInfo));

		addon->ModuleSize = moduleInfo.SizeOfImage;

		typedef int (*addextension2)(HINSTANCE);
		addextension2 exp_addextension2 = nullptr;

		if (addon->Definitions->Signature == 0xFFF694D1)
		{
			if (true == FindFunction(addon->Module, &exp_addextension2, "addextension2"))
			{
				LPVOID res{}; DWORD sz{};
				GetResource(NexusHandle, MAKEINTRESOURCE(RES_ARCDPS_INTEGRATION), "DLL", &res, &sz);

				try
				{
					if (std::filesystem::exists(Path::F_ARCDPSINTEGRATION))
					{
						std::filesystem::remove(Path::F_ARCDPSINTEGRATION);
					}

					std::ofstream file(Path::F_ARCDPSINTEGRATION, std::ios::binary);
					file.write((const char*)res, sz);
					file.close();

					// reload is set to true because the dll is always deployed from resource
					// and since it's locked it would not load here directly but instead after checking for updates (which does not apply to it)
					LoadAddon(Path::F_ARCDPSINTEGRATION, true);
				}
				catch (std::filesystem::filesystem_error fErr)
				{
					LogDebug(CH_LOADER, "%s", fErr.what());
					return;
				}
			}
			else
			{
				LogWarning(CH_LOADER, "Addon with signature \"0xFFF694D1\" found but \"addextension2\" is not exported. ArcDPS combat events won't be relayed.");
			}
		}
		
		addon->Definitions->Load(api);
		Events::Raise(EV_ADDON_LOADED, &addon->Definitions->Signature);
		Events::Raise(EV_MUMBLE_IDENTITY_UPDATED, MumbleIdentity);

		bool locked = addon->Definitions->Unload == nullptr || addon->Definitions->HasFlag(EAddonFlags::DisableHotloading);
		addon->State = locked ? EAddonState::LoadedLOCKED : EAddonState::Loaded;

		if (addon->Definitions->APIVersion == 0)
		{
			LogInfo(CH_LOADER, "Loaded addon: %s (Signature %d) [%p - %p] (No API was requested.)", strPath, addon->Definitions->Signature, addon->Module, ((PBYTE)addon->Module) + moduleInfo.SizeOfImage);
		}
		else
		{
			LogInfo(CH_LOADER, "Loaded addon: %s (Signature %d) [%p - %p] (API Version %d was requested.)", strPath, addon->Definitions->Signature, addon->Module, ((PBYTE)addon->Module) + moduleInfo.SizeOfImage, addon->Definitions->APIVersion);
		}

		if (exp_addextension2)
		{
			auto it = Addons.find(Path::F_ARCDPSINTEGRATION);

			if (it != Addons.end())
			{
				if (it->second->Module)
				{
					int result = exp_addextension2(it->second->Module);
					LogInfo(CH_LOADER, "Deployed ArcDPS Integration. Result: %d", result);
				}
				else
				{
					LogWarning(CH_LOADER, "ArcDPS Integration module was null.");
				}
			}
		}
	}
	void UnloadAddon(const std::filesystem::path& aPath, bool aIsShutdown)
	{
		std::string path = aPath.string();
		const char* strPath = path.c_str();

		auto it = Addons.find(aPath);

		if (it == Addons.end())
		{
			return;
		}

		Addon* addon = it->second;

		if (addon->State == EAddonState::NotLoaded ||
			addon->State == EAddonState::NotLoadedDuplicate ||
			addon->State == EAddonState::NotLoadedIncompatible ||
			addon->State == EAddonState::NotLoadedIncompatibleAPI)
		{
			//LogWarning(CH_LOADER, "Cancelled unload of \"%s\". EAddonState = %d.", strPath, addon->State);
			return;
		}

		if (addon->Definitions)
		{
			if (addon->State == EAddonState::Loaded)
			{
				addon->Definitions->Unload();
				Events::Raise(EV_ADDON_UNLOADED, &addon->Definitions->Signature);
			}
			else if (addon->State == EAddonState::LoadedLOCKED && aIsShutdown)
			{
				if (addon->Definitions->Unload)
				{
					/* If it's a shutdown and Unload is defined, let the addon run its shutdown routine to save settings etc, but do not freelib */
					addon->Definitions->Unload();
					Events::Raise(EV_ADDON_UNLOADED, &addon->Definitions->Signature);
				}
			}
		}

		if (addon->Module)
		{
			if (addon->State == EAddonState::Loaded ||
				(addon->State == EAddonState::LoadedLOCKED && aIsShutdown))
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
			}

			if (addon->State == EAddonState::Loaded)
			{
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

				//LogDebug(CH_LOADER, "Called FreeLibrary() %d times on \"%s\".", freeCalls, strPath);
			}
		}

		if (addon->State == EAddonState::Loaded)
		{
			addon->Module = nullptr;
			addon->ModuleSize = 0;

			addon->State = EAddonState::NotLoaded;

			if (!std::filesystem::exists(aPath))
			{
				Addons.erase(aPath);
			}

			LogInfo(CH_LOADER, "Unloaded addon: %s", strPath);
		}
		else if (addon->State == EAddonState::LoadedLOCKED && aIsShutdown)
		{
			LogInfo(CH_LOADER, "Unloaded addon on shutdown without freeing module due to locked state: %s", strPath);
		}
	}
	void UninstallAddon(const std::filesystem::path& aPath)
	{
		UnloadAddon(aPath);

		/* check both LoadedLOCKED, but also Loaded as a sanity check */
		auto it = Addons.find(aPath);

		/*
		if it's still loaded due to being locked (or for some obscure other reason)
		try to move addon.dll to addon.dll.old, so it will be deleted on next restart
		*/
		if (it != Addons.end())
		{
			if (it->second->State == EAddonState::Loaded || it->second->State == EAddonState::LoadedLOCKED)
			{
				try
				{
					std::filesystem::rename(aPath, aPath.string() + extUninstall);
					Addons.erase(aPath); // remove from addons list anyway
					LogWarning(CH_LOADER, "Addon is stilled loaded, it will be uninstalled the next time the game is restarted: %s", aPath.string().c_str());
				}
				catch (std::filesystem::filesystem_error fErr)
				{
					LogDebug(CH_LOADER, "%s", fErr.what());
					return;
				}

				return;
			}
		}

		// if file exists, delete it
		if (std::filesystem::exists(aPath))
		{
			try
			{
				std::filesystem::remove(aPath.string().c_str());
				Addons.erase(aPath);
				LogInfo(CH_LOADER, "Uninstalled addon: %s", aPath.string().c_str());
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				LogDebug(CH_LOADER, "%s", fErr.what());
				return;
			}
		}
	}
	void ReloadAddon(const std::filesystem::path& aPath)
	{
		UnloadAddon(aPath);
		LoadAddon(aPath, true);
	}
	void UpdateSwapAddon(const std::filesystem::path& aPath)
	{
		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + extOld;
		std::filesystem::path pathUpdate = aPath.string() + extUpdate;

		if (std::filesystem::exists(pathUpdate))
		{
			try
			{
				if (std::filesystem::exists(pathOld))
				{
					std::filesystem::remove(pathOld);
				}

				std::filesystem::rename(aPath, pathOld);
				std::filesystem::rename(pathUpdate, aPath);

				if (std::filesystem::exists(pathOld))
				{
					std::filesystem::remove(pathOld);
				}
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				LogDebug(CH_LOADER, "%s", fErr.what());
				return;
			}
		}
	}
	bool UpdateAddon(const std::filesystem::path& aPath, signed int aSignature, std::string aName, AddonVersion aVersion, EUpdateProvider aProvider, std::string aUpdateLink)
	{
		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + extOld;
		std::filesystem::path pathUpdate = aPath.string() + extUpdate;

		auto it = Addons.find(aPath);

		if (it == Addons.end())
		{
			return false;
		}

		Addon* addon = (*it).second;

		/* cleanup old files */
		try
		{
			if (std::filesystem::exists(pathOld)) { std::filesystem::remove(pathOld); }
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			LogDebug(CH_LOADER, "%s", fErr.what());
			return true; // report as update here, as it was probably moved here during runtime but the dll is locked
		}
		if (std::filesystem::exists(pathUpdate))
		{
			if (addon->MD5 != MD5FromFile(pathUpdate))
			{
				return true;
			}

			try
			{
				std::filesystem::remove(pathUpdate);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				LogDebug(CH_LOADER, "%s", fErr.what());
				return false;
			}
		}

		bool wasUpdated = false;

		std::string baseUrl;
		std::string endpoint;

		// override provider if none set, but a Raidcore ID is used
		if (aProvider == EUpdateProvider::None && aSignature > 0)
		{
			aProvider = EUpdateProvider::Raidcore;
		}

		/* setup baseUrl and endpoint */
		switch (aProvider)
		{
		case EUpdateProvider::None: return false;

		case EUpdateProvider::Raidcore:
			baseUrl = API_RAIDCORE;
			endpoint = "/addons/" + std::to_string(aSignature);

			break;

		case EUpdateProvider::GitHub:
			baseUrl = API_GITHUB;
			if (aUpdateLink.empty())
			{
				LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", aName.c_str());
				return false;
			}

			endpoint = "/repos" + GetEndpoint(aUpdateLink) + "/releases"; // "/releases/latest"; // fuck you Sognus

			break;

		case EUpdateProvider::Direct:
			if (aUpdateLink.empty())
			{
				LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aName.c_str());
				return false;
			}

			baseUrl = GetBaseURL(aUpdateLink);
			endpoint = GetEndpoint(aUpdateLink);

			if (baseUrl.empty() || endpoint.empty())
			{
				return false;
			}

			break;
		}

		if (EUpdateProvider::Raidcore == aProvider)
		{
			json resVersion = RaidcoreAPI->Get(endpoint);

			if (resVersion.is_null())
			{
				LogWarning(CH_LOADER, "Error parsing API response.");
				return false;
			}

			AddonVersion remoteVersion = VersionFromJson(resVersion);

			if (remoteVersion > aVersion)
			{
				LogInfo(CH_LOADER, "%s is outdated: API replied with Version %s but installed is Version %s", aName.c_str(), remoteVersion.ToString().c_str(), aVersion.ToString().c_str());

				RaidcoreAPI->Download(pathUpdate, endpoint + "/download"); // e.g. api.raidcore.gg/addons/17/download
				
				LogInfo(CH_LOADER, "Successfully updated %s.", aName.c_str());
				wasUpdated = true;
			}
		}
		else if (EUpdateProvider::GitHub == aProvider)
		{
			json response = GitHubAPI->Get(endpoint);

			if (response.is_null())
			{
				LogWarning(CH_LOADER, "Error parsing API response.");
				return false;
			}
			
			response = response[0]; // filthy hack to get "latest"

			if (response["tag_name"].is_null())
			{
				LogWarning(CH_LOADER, "No tag_name set on %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			std::string tagName = response["tag_name"].get<std::string>();

			if (!std::regex_match(tagName, std::regex("v?\\d+[.]\\d+[.]\\d+[.]\\d+")))
			{
				LogWarning(CH_LOADER, "tag_name on %s%s does not match convention e.g. \"1.0.0.1\" or \"v1.0.0.1\". Cannot check against version.", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			if (tagName._Starts_with("v"))
			{
				tagName = tagName.substr(1);
			}

			AddonVersion remoteVersion{};

			size_t pos = 0;
			int i = 0;
			while ((pos = tagName.find(".")) != std::string::npos)
			{
				switch (i)
				{
				case 0: remoteVersion.Major = static_cast<unsigned short>(std::stoi(tagName.substr(0, pos))); break;
				case 1: remoteVersion.Minor = static_cast<unsigned short>(std::stoi(tagName.substr(0, pos))); break;
				case 2: remoteVersion.Build = static_cast<unsigned short>(std::stoi(tagName.substr(0, pos))); break;
				}
				i++;
				tagName.erase(0, pos + 1);
			}
			remoteVersion.Revision = static_cast<unsigned short>(std::stoi(tagName));

			if (remoteVersion > aVersion)
			{
				LogInfo(CH_LOADER, "%s is outdated: API replied with Version %s but installed is Version %s", aName.c_str(), remoteVersion.ToString().c_str(), aVersion.ToString().c_str());

				std::string endpointDownload; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll

				if (response["assets"].is_null())
				{
					LogWarning(CH_LOADER, "Release has no assets. Cannot check against version. (%s%s)", baseUrl.c_str(), endpoint.c_str());
					return false;
				}

				for (auto& asset : response["assets"])
				{
					std::string assetName = asset["name"].get<std::string>();

					if (String::Contains(assetName, ".dll"))
					{
						asset["browser_download_url"].get_to(endpointDownload);
						break;
					}
				}

				std::string downloadBaseUrl = GetBaseURL(endpointDownload);
				endpointDownload = GetEndpoint(endpointDownload);

				httplib::Client downloadClient(downloadBaseUrl);
				downloadClient.enable_server_certificate_verification(false);
				downloadClient.set_follow_location(true);

				size_t bytesWritten = 0;
				std::ofstream file(pathUpdate, std::ofstream::binary);
				auto downloadResult = downloadClient.Get(endpointDownload, [&](const char* data, size_t data_length) {
					file.write(data, data_length);
					bytesWritten += data_length;
					return true;
					});
				file.close();

				if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
				{
					LogWarning(CH_LOADER, "Error fetching %s%s", downloadBaseUrl.c_str(), endpointDownload.c_str());
					return false;
				}

				LogInfo(CH_LOADER, "Successfully updated %s.", aName.c_str());
				wasUpdated = true;
			}
		}
		else if (EUpdateProvider::Direct == aProvider)
		{
			/* prepare client request */
			httplib::Client client(baseUrl);
			client.enable_server_certificate_verification(false);

			std::string endpointMD5 = endpoint + ".md5sum";

			std::ifstream fileCurrent(aPath, std::ios::binary);
			fileCurrent.seekg(0, std::ios::end);
			size_t length = fileCurrent.tellg();
			fileCurrent.seekg(0, std::ios::beg);
			char* buffer = new char[length];
			fileCurrent.read(buffer, length);

			std::vector<unsigned char> md5current = MD5((const unsigned char*)buffer, length);
			std::vector<unsigned char> md5remote;

			auto resultMd5Req = client.Get(endpointMD5, [&](const char* data, size_t data_length) {
				for (size_t i = 0; i < data_length; i += 2)
				{
					if (md5current.size() == md5remote.size())
					{
						break; // more bytes aren't needed
					}

					std::string str{};
					str += data[i];
					str += data[i + 1];

					unsigned char byte = (unsigned char)strtol(str.c_str(), NULL, 16);

					md5remote.push_back(byte);
				}
				return true;
				});

			delete[] buffer;

			if (!resultMd5Req || resultMd5Req->status != 200)
			{
				LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			if (md5current == md5remote)
			{
				return false;
			}

			auto lmHeader = resultMd5Req->headers.find("Last-Modified");

			if (lmHeader != resultMd5Req->headers.end())
			{
				long long remoteTimestamp = LastModifiedToTimestamp(lmHeader->second);

				std::filesystem::path timeOffsetFile = aPath.parent_path() / (aPath.stem().string() + ".0");
				std::ofstream file(timeOffsetFile);
				file << "0" << std::endl;
				file.close();

				long long timeOffset = Timestamp() - std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(timeOffsetFile).time_since_epoch()).count();
				std::filesystem::remove(timeOffsetFile);

				long long lastWriteTime = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(aPath).time_since_epoch()).count();
				if (lastWriteTime + timeOffset > remoteTimestamp)
				{
					return false;
				}
			}

			size_t bytesWritten = 0;
			std::ofstream fileUpdate(pathUpdate, std::ofstream::binary);
			auto downloadResult = client.Get(endpoint, [&](const char* data, size_t data_length) {
				fileUpdate.write(data, data_length);
				bytesWritten += data_length;
				return true;
				});
			fileUpdate.close();

			if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
			{
				LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			wasUpdated = true;
		}

		return wasUpdated;
	}
	void InstallAddon(LibraryAddon* aAddon)
	{
		aAddon->IsInstalling = true;

		/* this is all modified duplicate code from update */
		std::string baseUrl;
		std::string endpoint;

		// override provider if none set, but a Raidcore ID is used
		if (aAddon->Provider == EUpdateProvider::None && aAddon->Signature > 0)
		{
			aAddon->Provider = EUpdateProvider::Raidcore;
		}

		/* setup baseUrl and endpoint */
		switch (aAddon->Provider)
		{
		case EUpdateProvider::None: return;

		case EUpdateProvider::Raidcore:
			baseUrl = API_RAIDCORE;
			endpoint = "/addons/" + std::to_string(aAddon->Signature);

			break;

		case EUpdateProvider::GitHub:
			baseUrl = API_GITHUB;
			if (aAddon->DownloadURL.empty())
			{
				LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", aAddon->Name);
				return;
			}

			endpoint = "/repos" + GetEndpoint(aAddon->DownloadURL) + "/releases"; // "/releases/latest"; // fuck you Sognus

			break;

		case EUpdateProvider::Direct:
			if (aAddon->DownloadURL.empty())
			{
				LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aAddon->Name);
				return;
			}

			baseUrl = GetBaseURL(aAddon->DownloadURL);
			endpoint = GetEndpoint(aAddon->DownloadURL);

			if (baseUrl.empty() || endpoint.empty())
			{
				return;
			}

			break;
		}

		if (EUpdateProvider::Raidcore == aAddon->Provider)
		{
			LogWarning(CH_LOADER, "Downloading via Raidcore is not implemented yet, due to user-friendly names requiring an API request. If you see this tell the developers about it! Thank you!");
			return;
			//RaidcoreAPI->Download(addonPath, endpoint + "/download"); // e.g. api.raidcore.gg/addons/17/download
		}
		else if (EUpdateProvider::GitHub == aAddon->Provider)
		{
			json response = GitHubAPI->Get(endpoint);

			if (response.is_null())
			{
				LogWarning(CH_LOADER, "Error parsing API response.");
				return;
			}

			response = response[0]; // filthy hack to get "latest"

			if (response["tag_name"].is_null())
			{
				LogWarning(CH_LOADER, "No tag_name set on %s%s", baseUrl.c_str(), endpoint.c_str());
				return;
			}

			std::string endpointDownload; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll

			if (response["assets"].is_null())
			{
				LogWarning(CH_LOADER, "Release has no assets. Cannot check against version. (%s%s)", baseUrl.c_str(), endpoint.c_str());
				return;
			}

			for (auto& asset : response["assets"])
			{
				std::string assetName = asset["name"].get<std::string>();

				if (assetName.size() < 4)
				{
					continue;
				}

				if (std::string_view(assetName).substr(assetName.size() - 4) == ".dll")
				{
					asset["browser_download_url"].get_to(endpointDownload);
				}
			}

			std::string downloadBaseUrl = GetBaseURL(endpointDownload);
			endpointDownload = GetEndpoint(endpointDownload);

			httplib::Client downloadClient(downloadBaseUrl);
			downloadClient.enable_server_certificate_verification(false);
			downloadClient.set_follow_location(true);

			size_t lastSlashPos = endpointDownload.find_last_of('/');
			std::string filename = endpointDownload.substr(lastSlashPos + 1);
			size_t dotDllPos = filename.find(extDll);
			filename = filename.substr(0, filename.length() - extDll.length());

			std::filesystem::path probe = Path::D_GW2_ADDONS / (filename + extDll);

			int i = 0;
			while (std::filesystem::exists(probe))
			{
				probe = Path::D_GW2_ADDONS / (filename + "_" + std::to_string(i) + extDll);
				i++;
			}

			size_t bytesWritten = 0;
			std::ofstream file(probe, std::ofstream::binary);
			auto downloadResult = downloadClient.Get(endpointDownload, [&](const char* data, size_t data_length) {
				file.write(data, data_length);
				bytesWritten += data_length;
				return true;
				});
			file.close();

			if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
			{
				LogWarning(CH_LOADER, "Error fetching %s%s", downloadBaseUrl.c_str(), endpointDownload.c_str());
				return;
			}
		}
		else if (EUpdateProvider::Direct == aAddon->Provider)
		{
			/* prepare client request */
			httplib::Client client(baseUrl);
			client.enable_server_certificate_verification(false);

			size_t lastSlashPos = endpoint.find_last_of('/');
			std::string filename = endpoint.substr(lastSlashPos + 1);
			size_t dotDllPos = filename.find(extDll);
			filename = filename.substr(0, filename.length() - extDll.length());

			std::filesystem::path probe = Path::D_GW2_ADDONS / (filename + extDll);

			int i = 0;
			while (std::filesystem::exists(probe))
			{
				probe = Path::D_GW2_ADDONS / (filename + "_" + std::to_string(i) + extDll);
				i++;
			}

			size_t bytesWritten = 0;
			std::ofstream fileUpdate(probe, std::ofstream::binary);
			auto downloadResult = client.Get(endpoint, [&](const char* data, size_t data_length) {
				fileUpdate.write(data, data_length);
				bytesWritten += data_length;
				return true;
				});
			fileUpdate.close();

			if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
			{
				LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return;
			}
		}

		LogInfo(CH_LOADER, "Successfully installed %s.", aAddon->Name.c_str());
		NotifyChanges();
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
			((AddonAPI1*)api)->DeregisterRender = GUI::Deregister;

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
			((AddonAPI1*)api)->DeregisterWndProc = WndProc::Deregister;

			((AddonAPI1*)api)->RegisterKeybindWithString = Keybinds::Register;
			((AddonAPI1*)api)->RegisterKeybindWithStruct = Keybinds::RegisterWithStruct;
			((AddonAPI1*)api)->DeregisterKeybind = Keybinds::Deregister;

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
			((AddonAPI2*)api)->DeregisterRender = GUI::Deregister;

			((AddonAPI2*)api)->GetGameDirectory = Path::GetGameDirectory;
			((AddonAPI2*)api)->GetAddonDirectory = Path::GetAddonDirectory;
			((AddonAPI2*)api)->GetCommonDirectory = Path::GetCommonDirectory;

			((AddonAPI2*)api)->CreateHook = MH_CreateHook;
			((AddonAPI2*)api)->RemoveHook = MH_RemoveHook;
			((AddonAPI2*)api)->EnableHook = MH_EnableHook;
			((AddonAPI2*)api)->DisableHook = MH_DisableHook;

			((AddonAPI2*)api)->Log = LogMessageAddon2;

			((AddonAPI2*)api)->RaiseEvent = Events::Raise;
			((AddonAPI2*)api)->RaiseEventNotification = Events::RaiseNotification;
			((AddonAPI2*)api)->SubscribeEvent = Events::Subscribe;
			((AddonAPI2*)api)->UnsubscribeEvent = Events::Unsubscribe;

			((AddonAPI2*)api)->RegisterWndProc = WndProc::Register;
			((AddonAPI2*)api)->DeregisterWndProc = WndProc::Deregister;

			((AddonAPI2*)api)->RegisterKeybindWithString = Keybinds::Register;
			((AddonAPI2*)api)->RegisterKeybindWithStruct = Keybinds::RegisterWithStruct;
			((AddonAPI2*)api)->DeregisterKeybind = Keybinds::Deregister;

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

	void CopyAddonDefs(AddonDefinition* aDefinitions, AddonDefinition** aOutDefinitions)
	{
		if (aDefinitions == nullptr)
		{
			*aOutDefinitions = new AddonDefinition{};
			return;
		}

		// Allocate new memory and copy data, copy strings
		*aOutDefinitions = new AddonDefinition(*aDefinitions);
		(*aOutDefinitions)->Name = _strdup(aDefinitions->Name);
		(*aOutDefinitions)->Author = _strdup(aDefinitions->Author);
		(*aOutDefinitions)->Description = _strdup(aDefinitions->Description);
		(*aOutDefinitions)->UpdateLink = aDefinitions->UpdateLink ? _strdup(aDefinitions->UpdateLink) : nullptr;
	}
	void FreeAddonDefs(AddonDefinition** aDefinitions)
	{
		if (*aDefinitions == nullptr) { return; }

		free((char*)(*aDefinitions)->Name);
		free((char*)(*aDefinitions)->Author);
		free((char*)(*aDefinitions)->Description);
		if ((*aDefinitions)->UpdateLink)
		{
			free((char*)(*aDefinitions)->UpdateLink);
		}
		delete *aDefinitions;

		*aDefinitions = nullptr;
	}

	std::string GetOwner(void* aAddress)
	{
		if (aAddress == nullptr)
		{
			return "(null)";
		}

		//const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (auto& [path, addon] : Addons)
			{
				if (!addon->Module) { continue; }

				void* startAddress = addon->Module;
				void* endAddress = ((PBYTE)addon->Module) + addon->ModuleSize;

				if (aAddress >= startAddress && aAddress <= endAddress)
				{
					return addon->Definitions ? addon->Definitions->Name : "(null)";
				}
			}

			void* startAddress = NexusHandle;
			void* endAddress = ((PBYTE)NexusHandle) + NexusModuleSize;

			if (aAddress >= startAddress && aAddress <= endAddress)
			{
				return "Nexus";
			}
		}

		return "(null)";
	}
}