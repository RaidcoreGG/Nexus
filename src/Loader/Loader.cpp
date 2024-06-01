///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.cpp
/// Description  :  Handles addon hot-loading, updates etc.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Loader.h"

#include <Windows.h>
#include <Psapi.h>
#include <regex>
#include <vector>
#include <chrono>

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
#include "GUI/Widgets/Alerts/Alerts.h"
#include "Settings/Settings.h"
#include "Localization/Localization.h"
#include "Updater/Updater.h"

#include "ArcDPS.h"
#include "Library.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "httplib/httplib.h"

#define LOADER_WAITTIME_MS 100

namespace Loader
{
	std::mutex					Mutex;
	std::unordered_map<
		std::filesystem::path,
		ELoaderAction
	>							QueuedAddons;
	std::vector<Addon*>			Addons;
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

	std::vector<signed int>		WhitelistedAddons;				/* List of addons that should be loaded on initial startup. */

	bool						DisableVolatileUntilUpdate	= false;

	void Initialize()
	{
		if (State::Nexus == ENexusState::LOADED)
		{
			LoadAddonConfig();

			//FSItemList = ILCreateFromPathA(Path::GetAddonDirectory(nullptr));
			std::wstring addonDirW = StrToWStr(Path::D_GW2_ADDONS.string());
			HRESULT hresult = SHParseDisplayName(
				addonDirW.c_str(),
				0,
				&FSItemList,
				0xFFFFFFFF,
				0
			);
			if (FSItemList == 0)
			{
				LogCritical(CH_LOADER, "Loader disabled. Reason: SHParseDisplayName(Path::D_GW2_ADDONS) returned %d.", hresult);
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

			std::thread(Library::Fetch).detach();
			std::thread(ArcDPS::GetPluginLibrary).detach();

			std::thread checkLaunchSequence([]()
				{
					Sleep(5000);
					int nothingCounter = 0;
					while (!IsGameplay)
					{
						/* do nothing */
						Sleep(1);
						nothingCounter++;

						if (nothingCounter > 30000)
						{
							break;
						}
					}
					IsGameLaunchSequence = false;
				});
			checkLaunchSequence.detach();

			LoaderThread = std::thread(ProcessChanges);
			LoaderThread.detach();
		}
	}
	void Shutdown()
	{
		if (State::Nexus == ENexusState::SHUTTING_DOWN)
		{
			bool loaderWasWorking = FSNotifierID != 0;

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
			if (loaderWasWorking)
			{
				SaveAddonConfig();
			}

			{
				while (Addons.size() != 0)
				{
					UnloadAddon(Addons.front()->Path);
					Addons.erase(Addons.begin());
				}
			}
		}
	}

	void LoadAddonConfig()
	{
		if (std::filesystem::exists(Path::F_ADDONCONFIG))
		{
			try
			{
				std::ifstream file(Path::F_ADDONCONFIG);

				json cfg = json::parse(file);
				for (json addonInfo : cfg)
				{
					signed int signature = 0;
					if (!addonInfo["Signature"].is_null()) { addonInfo["Signature"].get_to(signature); }

					if (signature == 0) { continue; }

					Addon* addon = FindAddonBySig(signature);

					if (!addon)
					{
						addon = new Addon{};
						addon->State = EAddonState::None;
						Addons.push_back(addon);
					}

					if (!addonInfo["IsPausingUpdates"].is_null()) { addonInfo["IsPausingUpdates"].get_to(addon->IsPausingUpdates); }
					if (!addonInfo["IsDisabledUntilUpdate"].is_null()) { addonInfo["IsDisabledUntilUpdate"].get_to(addon->IsDisabledUntilUpdate); }

					// to match the actual addon to the saved states
					addon->MatchSignature = signature;

					/* should load, indicates whether it was loaded last time */
					bool shouldLoad = false;
					if (!addonInfo["IsLoaded"].is_null())
					{
						shouldLoad = addonInfo["IsLoaded"].get<bool>() && !addon->IsDisabledUntilUpdate;
					}

					if (shouldLoad)
					{
						auto it = std::find(WhitelistedAddons.begin(), WhitelistedAddons.end(), signature);
						if (it == WhitelistedAddons.end())
						{
							WhitelistedAddons.push_back(signature);
						}
					}
				}

				file.close();
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_KEYBINDS, "AddonConfig.json could not be parsed. Error: %s", ex.what());
			}
		}

		/* if addons were specified via param, only load those */
		if (RequestedAddons.size() > 0)
		{
			WhitelistedAddons.clear();
			WhitelistedAddons = RequestedAddons;
		}

		/* ensure arcdps integration will be loaded */
		auto hasArcIntegration = std::find(WhitelistedAddons.begin(), WhitelistedAddons.end(), 0xFED81763);
		if (hasArcIntegration == WhitelistedAddons.end())
		{
			WhitelistedAddons.push_back(0xFED81763);
		}
	}
	void SaveAddonConfig()
	{
		// don't save state if state was overridden via start param
		if (RequestedAddons.size() > 0) { return; }

		// don't save state if we're shutting down
		if (State::Nexus >= ENexusState::SHUTTING_DOWN) { return; }

		json cfg = json::array();

		std::vector<signed int> trackedSigs;

		for (auto addon : Addons)
		{
			signed int sig = addon->Definitions ? addon->Definitions->Signature : addon->MatchSignature;

			// skip bridge
			if (sig == 0xFED81763) { continue; }
			if (sig == 0) { continue; }

			auto tracked = std::find(trackedSigs.begin(), trackedSigs.end(), sig);
			if (tracked != trackedSigs.end()) { continue; }

			json addonInfo =
			{
				{"Signature", sig},
				{"IsLoaded", addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED || addon->IsFlaggedForEnable},
				{"IsPausingUpdates", addon->IsPausingUpdates},
				{"IsDisabledUntilUpdate", addon->IsDisabledUntilUpdate}
			};

			/* override loaded state, if it's still the initial startup sequence */
			if (addon->State == EAddonState::None)
			{
				if (std::find(WhitelistedAddons.begin(), WhitelistedAddons.end(), sig) != WhitelistedAddons.end())
				{
					addonInfo["IsLoaded"] = true;
				}
			}

			/* override loaded state, if it's supposed to disable next launch */
			if (addon->IsFlaggedForDisable)
			{
				addonInfo["IsLoaded"] = false;
			}

			cfg.push_back(addonInfo);
		}

		std::ofstream file(Path::F_ADDONCONFIG);
		file << cfg.dump(1, '\t') << std::endl;
		file.close();
	}

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_ADDONDIRUPDATE)
		{
			PIDLIST_ABSOLUTE* ppidl;
			LONG event;
			HANDLE notificationLock = SHChangeNotification_Lock((HANDLE)wParam, static_cast<DWORD>(lParam), &ppidl, &event);

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
				UnloadAddon(it->first);
				UninstallAddon(it->first);
				break;
			case ELoaderAction::Reload:
				/* if it's already loaded, the unload call with unload, then load async (after done)
				 * if it's not already loaded, the unload call is skipped, and it's loaded instead */

				UnloadAddon(it->first, true);
				LoadAddon(it->first, true);
				break;
			case ELoaderAction::FreeLibrary:
				FreeAddon(it->first);
				break;

			// this can only be invoked via UnloadAddon(..., true) (aka Reload)
			case ELoaderAction::FreeLibraryThenLoad:
				FreeAddon(it->first);
				LoadAddon(it->first, true);
				break;
			}
			
			QueuedAddons.erase(it);

			SortAddons();
			ArcDPS::GetPlugins();
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
		/* fetch game version before loading addons */
		GetGameBuild();

		for (;;)
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

			const std::lock_guard<std::mutex> lock(Mutex);
			// check all tracked addons
			for (Addon* addon : Addons)
			{
				// if addon no longer on disk (also check if the path is not null, else it's from config)
				if (!std::filesystem::exists(addon->Path) && addon->Path != "")
				{
					QueueAddon(ELoaderAction::Unload, addon->Path);
					continue;
				}

				// get md5 of each file currently on disk and compare to tracked md5
				// also check if an update is available (e.g. "addon.dll" + ".update" -> "addon.dll.update" exists)
				std::vector<unsigned char> md5 = MD5Util::FromFile(addon->Path);
				std::filesystem::path updatePath = addon->Path.string() + extUpdate;
				if ((addon->MD5.empty() || addon->MD5 != md5) || std::filesystem::exists(updatePath))
				{
					UpdateSwapAddon(addon->Path);

					// only reload if it already is loaded
					if (addon->State == EAddonState::Loaded)
					{
						QueueAddon(ELoaderAction::Reload, addon->Path);
					}
				}
			}

			// check all other files in the directory
			for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
			{
				std::filesystem::path path = entry.path();

				if (std::filesystem::is_directory(path))
				{
					continue;
				}

				// if already tracked
				Addon* exists = FindAddonByPath(path);
				if (exists)
				{
					continue;
				}

				if (std::filesystem::is_symlink(path))
				{
					std::filesystem::path realLocation = std::filesystem::read_symlink(path);

					if (std::filesystem::is_directory(realLocation) ||
						!std::filesystem::exists(realLocation) ||
						std::filesystem::file_size(realLocation) == 0)
					{
						continue;
					}
				}

				if (std::filesystem::file_size(path) == 0)
				{
					continue;
				}

				if (path.extension() == extDll)
				{
					std::vector<unsigned char> md5 = MD5Util::FromFile(path);
					if (FindAddonByMD5(md5) == nullptr)
					{
						QueueAddon(ELoaderAction::Load, path);
					}
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
					}
				}
				else if (path.extension() == extOld)
				{
					std::filesystem::path actualPath = path.string().substr(0, path.string().length() - extOld.length());

					Addon* addon = FindAddonByPath(actualPath);

					/* not orphaned, will be handled by loader */
					if (addon) { continue; }

					try
					{
						std::filesystem::remove(path);
					}
					catch (std::filesystem::filesystem_error fErr)
					{
						LogDebug(CH_LOADER, "%s", fErr.what());
					}
				}
				else if (path.extension() == extUpdate)
				{
					std::filesystem::path actualPath = path.string().substr(0, path.string().length() - extUpdate.length());

					Addon* addon = FindAddonByPath(actualPath);

					/* not orphaned, will be handled by loader */
					if (addon) { continue; }

					try
					{
						std::filesystem::rename(path, actualPath);
					}
					catch (std::filesystem::filesystem_error fErr)
					{
						LogDebug(CH_LOADER, "%s", fErr.what());
					}
				}
			}
			
			IsSuspended = true;
		}
	}
	
	void LoadAddon(const std::filesystem::path& aPath, bool aIsReload)
	{
		std::string path = aPath.string();
		std::string strFile = aPath.filename().string();

		/* used to indicate whether the addon already existed or was newly allocated and has to be merged (possibly) with the config-created one */
		bool allocNew = false;

		Addon* addon = FindAddonByPath(aPath);

		if (addon)
		{
			if (addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED)
			{
				//LogWarning(CH_LOADER, "Cancelled loading \"%s\". Already loaded.", strFile.c_str());
				return;
			}
		}
		else
		{
			allocNew = true;
			addon = new Addon{};
			addon->State = EAddonState::None;
			addon->Path = aPath;
		}

		UpdateSwapAddon(aPath);

		GETADDONDEF getAddonDef = 0;
		addon->MD5 = MD5Util::FromFile(aPath);
		addon->Module = LoadLibraryA(path.c_str());

		/* load lib failed */
		if (!addon->Module)
		{
			LogWarning(CH_LOADER, "Failed LoadLibrary on \"%s\". Incompatible. Last Error: %u", strFile.c_str(), GetLastError());
			addon->State = EAddonState::NotLoadedIncompatible;

			if (allocNew)
			{
				Addons.push_back(addon); // track this anyway
			}

			return;
		}

		/* doesn't have GetAddonDef */
		if (FindFunction(addon->Module, &getAddonDef, "GetAddonDef") == false)
		{
			LogWarning(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Incompatible.", strFile.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;

			if (allocNew)
			{
				Addons.push_back(addon); // track this anyway
			}

			return;
		}
		
		AddonDefinition* tmpDefs = getAddonDef();

		/* addon defs are nullptr */
		if (tmpDefs == nullptr)
		{
			LogWarning(CH_LOADER, "\"%s\" is exporting \"GetAddonDef\" but returned a nullptr. Incompatible.", strFile.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;

			if (allocNew)
			{
				Addons.push_back(addon); // track this anyway
			}

			return;
		}

		/* free old (if exists) and clone new to show in list */
		AddonDefinition::Free(&addon->Definitions);
		AddonDefinition::Copy(tmpDefs, &addon->Definitions);

		/* get stored info about addon and apply to addon */
		if (allocNew)
		{
			Addon* stored = FindAddonByMatchSig(addon->Definitions->Signature);

			// stored exists and is "unclaimed"
			if (stored && stored->State == EAddonState::None)
			{
				/* we have some settings/info stored, we merge and delete the new alloc */
				Addon* alloc = addon;
				addon = stored;
				addon->Definitions = alloc->Definitions;
				addon->Path = aPath;
				addon->Module = alloc->Module;
				addon->State = alloc->State;
				addon->MD5 = alloc->MD5;

				delete alloc;
			}
			else
			{
				addon->MatchSignature = addon->Definitions->Signature;
				Addons.push_back(addon); // track this anyway
			}
		}

		/* check if duplicate signature */
		auto duplicate = std::find_if(Addons.begin(), Addons.end(), [addon](Addon* cmpAddon) {
			// check if it has definitions and if the signature is the same
			return	cmpAddon->Path != addon->Path &&
				cmpAddon->Definitions &&
				cmpAddon->Definitions->Signature == addon->Definitions->Signature;
			});
		if (duplicate != Addons.end()) {
			LogWarning(CH_LOADER, "\"%s\" or another addon with this signature (%d) is already loaded. Duplicate.", strFile.c_str(), addon->Definitions->Signature);
			FreeLibrary(addon->Module);
			AddonDefinition::Free(&addon->Definitions);
			addon->State = EAddonState::NotLoadedDuplicate;
			addon->Module = nullptr;
			return;
		}

		bool isInitialLoad = addon->State == EAddonState::None;

		// if not on whitelist and its the initial load (aka not manually invoked)
		auto it = std::find(WhitelistedAddons.begin(), WhitelistedAddons.end(), addon->Definitions->Signature);
		bool shouldLoad = it != WhitelistedAddons.end() || !isInitialLoad;

		// if pausing updates, but wasn't set to be disabled until update
		bool shouldCheckForUpdate = !(addon->IsPausingUpdates && !addon->IsDisabledUntilUpdate);

		/* set DUU state if game has updated and addon is volatile and this is the intial load */
		if (isInitialLoad && addon->Definitions->HasFlag(EAddonFlags::IsVolatile) && DisableVolatileUntilUpdate)
		{
			addon->IsDisabledUntilUpdate = true;
			SaveAddonConfig(); // save the DUU state
		}
		else if (!isInitialLoad && addon->IsDisabledUntilUpdate) // reset DUU state if loading manually
		{
			addon->IsDisabledUntilUpdate = false;
			SaveAddonConfig(); // save the DUU state
		}

		/* predeclare locked helper for later */
		bool locked = addon->Definitions->Unload == nullptr || addon->Definitions->HasFlag(EAddonFlags::DisableHotloading);
		bool onlyInitialLaunch = addon->Definitions->HasFlag(EAddonFlags::OnlyLoadDuringGameLaunchSequence) || addon->Definitions->Signature == 0xFFF694D1;
		// FIXME: remove the arcdps check as soon as it adds the flag

		/* override shoudLoad */
		if (!IsGameLaunchSequence && onlyInitialLaunch)
		{
			shouldLoad = false;
		}

		/* don't update when reloading; check when: it's waiting to re-enable but wasn't manually invoked, it's not pausing updates atm */
		if (!aIsReload && ((addon->IsDisabledUntilUpdate && isInitialLoad) || !addon->IsPausingUpdates))
		{
			std::filesystem::path tmpPath = aPath.string();
			std::thread([tmpPath, addon, locked, shouldLoad, onlyInitialLaunch]()
				{
					bool lShouldLoad = shouldLoad;

					CUpdater& inst = CUpdater::GetInstance();

					AddonInfo addonInfo
					{
						addon->Definitions->Signature,
						addon->Definitions->Name,
						addon->Definitions->Version,
						addon->Definitions->Provider,
						addon->Definitions->UpdateLink != nullptr
							? addon->Definitions->UpdateLink
							: "",
						addon->MD5
					};

					if (inst.UpdateAddon(tmpPath, addonInfo))
					{
						LogInfo(CH_LOADER, "Update available for \"%s\".", tmpPath.string().c_str());
						if (addon->IsDisabledUntilUpdate)
						{
							// reset state, because it updated
							addon->IsDisabledUntilUpdate = false;
							lShouldLoad = true;
							addon->IsFlaggedForEnable = true;

							// mutex because we're async/threading
							{
								const std::lock_guard<std::mutex> lock(Mutex);
								SaveAddonConfig(); // save the DUU state
							}
						}

						/* only call reload if it wasn't unloaded */
						if (lShouldLoad)
						{
							if (!onlyInitialLaunch)
							{
								QueueAddon(ELoaderAction::Reload, tmpPath);
							}
							else
							{
								GUI::Alerts::Notify(String::Format("%s %s", addon->Definitions->Name, Language.Translate("((000079))")).c_str());
							}
						}
					}
					else if (locked && lShouldLoad && !addon->IsDisabledUntilUpdate) // if addon is locked and not DUU
					{
						// the lock state is checked because if it will be locked it means it was unloaded, prior to checking for an update

						/* only call reload if it wasn't unloaded */
						if (!onlyInitialLaunch)
						{
							QueueAddon(ELoaderAction::Reload, tmpPath);
						}
					}
				})
				.detach();

			/* if will be locked, but this addon must NOT only be loaded at startup, explicitly unload so the update can invoke a reload */
			if (locked && !onlyInitialLaunch)
			{
				FreeLibrary(addon->Module);
				addon->State = EAddonState::NotLoaded;
				addon->Module = nullptr;
				return;
			}
		}

		/* don't load addons that weren't requested or loaded last time (ignore arcdps integration) */
		if (!shouldLoad)
		{
			//LogInfo(CH_LOADER, "\"%s\" was not requested via start parameter or last state was disabled. Skipped.", strFile.c_str(), addon->Definitions->Signature);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoaded;
			addon->Module = nullptr;
			return;
		}

		if (addon->IsDisabledUntilUpdate && DisableVolatileUntilUpdate)
		{
			// show message that addon was disabled due to game update
			Events::Raise(EV_VOLATILE_ADDON_DISABLED, &addon->Definitions->Signature);
			GUI::Alerts::Notify(String::Format("%s %s", addon->Definitions->Name, Language.Translate("((000073))")).c_str());

			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoaded;
			addon->Module = nullptr;
			return;
		}

		/* doesn't fulfill min reqs */
		if (!addon->Definitions->HasMinimumRequirements())
		{
			LogWarning(CH_LOADER, "\"%s\" does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as the Load function. Incompatible.", strFile.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}

		/* (effectively duplicate check) if someone wants to do shenanigans and inject a different integration module */
		if (addon->Definitions->Signature == 0xFED81763 && aPath != Path::F_ARCDPSINTEGRATION)
		{
			LogWarning(CH_LOADER, "\"%s\" declares signature 0xFED81763 but is not the actual Nexus ArcDPS Integration. Either this was in error or an attempt to tamper with Nexus files. Incompatible.", strFile.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}

		AddonAPI* api = GetAddonAPI(addon->Definitions->APIVersion); // will be nullptr if doesn't exist or APIVersion = 0

		// if the api doesn't exist and there was one requested
		if (api == nullptr && addon->Definitions->APIVersion != 0)
		{
			LogWarning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Incompatible.", strFile.c_str(), addon->Definitions->APIVersion);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatibleAPI;
			addon->Module = nullptr;
			return;
		}

		MODULEINFO moduleInfo{};
		GetModuleInformation(GetCurrentProcess(), addon->Module, &moduleInfo, sizeof(moduleInfo));
		addon->ModuleSize = moduleInfo.SizeOfImage;

		auto start_time = std::chrono::high_resolution_clock::now();
		addon->Definitions->Load(api);
		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;

		Events::Raise(EV_ADDON_LOADED, &addon->Definitions->Signature);
		Events::Raise(EV_MUMBLE_IDENTITY_UPDATED, MumbleIdentity);

		SortAddons();

		addon->State = locked ? EAddonState::LoadedLOCKED : EAddonState::Loaded;
		SaveAddonConfig();

		LogInfo(CH_LOADER, u8"Loaded addon: %s (Signature %d) [%p - %p] (API Version %d was requested.) Took %uµs.", 
			strFile.c_str(), addon->Definitions->Signature,
			addon->Module, ((PBYTE)addon->Module) + moduleInfo.SizeOfImage,
			addon->Definitions->APIVersion, time / std::chrono::microseconds(1)
		);

		/* if arcdps */
		if (addon->Definitions->Signature == 0xFFF694D1)
		{
			ArcDPS::ModuleHandle = addon->Module;
			ArcDPS::IsLoaded = true;

			ArcDPS::DeployBridge();
		}
		else if (addon->Definitions->Signature == 0xFED81763 && ArcDPS::ModuleHandle) /* if arcdps bridge */
		{
			ArcDPS::InitializeBridge(addon->Module);
		}
	}

	void CallUnloadAndVerify(const std::filesystem::path& aPath, Addon* aAddon)
	{
		bool isShutdown = State::Nexus == ENexusState::SHUTTING_DOWN;

		if (!isShutdown)
		{
			/* cache the flag, save that the addon will disable, then restore it */
			bool flagDisable = aAddon->IsFlaggedForDisable;
			aAddon->IsFlaggedForDisable = true;
			
			/* wrap around mutex lock/unlock because if SyncUnload it will already be locked, as it's executed from ProcessQueue */
			if (!aAddon->Definitions->HasFlag(EAddonFlags::SyncUnload))
			{
				/* explictly lock the mutex, because we're unloading ASYNC -> threadsafe */
				const std::lock_guard<std::mutex> lock(Mutex);
				SaveAddonConfig();
			}
			else
			{
				/* do not explicitly lock the mutex, because we're unloading SYNC -> already locked in ProcessQueue */
				SaveAddonConfig();
			}

			aAddon->IsFlaggedForDisable = flagDisable;
		}

		std::chrono::steady_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		if (aAddon->Definitions->Unload) // guaranteed to have defs because only called from UnloadAddon()
		{
			aAddon->Definitions->Unload();
		}
		std::chrono::steady_clock::time_point end_time = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::duration time = end_time - start_time;

		if (aAddon->Module && aAddon->ModuleSize > 0)
		{
			/* Verify all APIs don't have any unreleased references to the addons address space */
			void* startAddress = aAddon->Module;
			void* endAddress = ((PBYTE)aAddon->Module) + aAddon->ModuleSize;

			int leftoverRefs = 0;
			leftoverRefs += Events::Verify(startAddress, endAddress);
			leftoverRefs += GUI::Verify(startAddress, endAddress);
			leftoverRefs += GUI::QuickAccess::Verify(startAddress, endAddress);
			leftoverRefs += Keybinds::Verify(startAddress, endAddress);
			leftoverRefs += WndProc::Verify(startAddress, endAddress);

			if (leftoverRefs > 0)
			{
				LogWarning(CH_LOADER, "Removed %d unreleased references from \"%s\". Make sure your addon releases all references during Addon::Unload().", leftoverRefs, aPath.filename().string().c_str());
			}
		}

		aAddon->IsWaitingForUnload = false;
		LogInfo(CH_LOADER, u8"Unloaded addon: %s (Took %uµs.)", aPath.filename().string().c_str(), time / std::chrono::microseconds(1));
	}

	void UnloadAddon(const std::filesystem::path& aPath, bool aDoReload)
	{
		std::string path = aPath.string();
		std::string strFile = aPath.filename().string();

		Addon* addon = FindAddonByPath(aPath);

		/* if the to be unloaded addon does not exist or isn't loaded (or loadedLocked) -> abort */
		if (!addon || !(addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED))
		{
			/* check if the addon exists because it could be null here */
			if (addon && !std::filesystem::exists(aPath))
			{
				auto it = std::find(Addons.begin(), Addons.end(), addon);
				if (it != Addons.end())
				{
					/* attempt freeing the addonDefs if the addon is still in memory */
					if (addon->Definitions)
					{
						AddonDefinition::Free(&addon->Definitions);
					}

					/* remove addon from list, it's no longer on disk*/
					Addons.erase(it);
				}
			}
			//LogWarning(CH_LOADER, "Cancelled unload of \"%s\". EAddonState = %d.", strFile.c_str(), addon->State);
			return;
		}

		bool isShutdown = State::Nexus == ENexusState::SHUTTING_DOWN;

		if (addon->Definitions)
		{
			// Either normal unload
			// or shutting down anyway, addon has Unload defined, so it can save settings etc
			if ((addon->State == EAddonState::Loaded) || (addon->State == EAddonState::LoadedLOCKED && isShutdown))
			{
				addon->IsWaitingForUnload = true;

				if (addon->Definitions->HasFlag(EAddonFlags::SyncUnload))
				{
					CallUnloadAndVerify(aPath, addon);

					if (!isShutdown)
					{
						FreeAddon(aPath);
					}

					if (aDoReload)
					{
						LoadAddon(aPath, true);
					}
				}
				else
				{
					std::thread unloadTask([addon, aPath, aDoReload, isShutdown]()
						{
							CallUnloadAndVerify(aPath, addon);

							if (!isShutdown)
							{
								Events::Raise(EV_ADDON_UNLOADED, &addon->Definitions->Signature);

								const std::lock_guard<std::mutex> lock(Mutex);
								if (aDoReload)
								{
									Loader::QueueAddon(ELoaderAction::FreeLibraryThenLoad, aPath);
								}
								else
								{
									Loader::QueueAddon(ELoaderAction::FreeLibrary, aPath);
								}
							}
						});
					unloadTask.detach();
				}
			}
		}
	}
	
	void FreeAddon(const std::filesystem::path& aPath)
	{
		std::string path = aPath.string();
		std::string strFile = aPath.filename().string();

		Addon* addon = FindAddonByPath(aPath);

		if (!addon)
		{
			return;
		}

		/* sanity check */
		if (addon->Module)
		{
			FreeLibrary(addon->Module);
		}

		addon->Module = nullptr;
		addon->ModuleSize = 0;

		addon->State = EAddonState::NotLoaded;

		SaveAddonConfig(); // no need to check if this is a shutdown, SaveAddonConfig checks that and prevents saving

		if (!std::filesystem::exists(aPath))
		{
			auto it = std::find(Addons.begin(), Addons.end(), addon);
			if (it != Addons.end())
			{
				if (addon->Definitions)
				{
					AddonDefinition::Free(&addon->Definitions);
				}
				Addons.erase(it);
			}
		}

		//LogDebug(CH_LOADER, "Called FreeLibrary() %d times on \"%s\".", freeCalls, strFile.c_str());

		Loader::NotifyChanges();
	}

	void UninstallAddon(const std::filesystem::path& aPath)
	{
		/* check both LoadedLOCKED, but also Loaded as a sanity check */
		Addon* addon = FindAddonByPath(aPath);

		/* if it's still loaded due to being locked (or for some obscure other reason)
		try to move addon.dll to addon.dll.uninstall, so it will be deleted on next restart */
		if (addon)
		{
			if (addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED || addon->IsWaitingForUnload)
			{
				try
				{
					std::filesystem::rename(aPath, aPath.string() + extUninstall);
					addon->IsFlaggedForUninstall = true;
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

				auto it = std::find(Addons.begin(), Addons.end(), addon);
				if (it != Addons.end())
				{
					if (addon && addon->Definitions)
					{
						AddonDefinition::Free(&addon->Definitions);
					}
					Addons.erase(it);
				}
				LogInfo(CH_LOADER, "Uninstalled addon: %s", aPath.string().c_str());
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				LogDebug(CH_LOADER, "%s", fErr.what());
				return;
			}
		}
	}

	bool UpdateSwapAddon(const std::filesystem::path& aPath)
	{
		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + extOld;
		std::filesystem::path pathUpdate = aPath.string() + extUpdate;

		/* remove leftover .old */
		if (std::filesystem::exists(pathOld))
		{
			try
			{
				std::filesystem::remove(pathOld);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				LogDebug(CH_LOADER, "%s", fErr.what());
				return false;
			}
		}

		if (std::filesystem::exists(pathUpdate))
		{
			try
			{
				std::filesystem::rename(aPath, pathOld);
				std::filesystem::rename(pathUpdate, aPath);

				return true;
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				LogDebug(CH_LOADER, "%s", fErr.what());
				return false;
			}
		}

		return false;
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

			((AddonAPI1*)api)->Log = ADDONAPI_LogMessage;

			((AddonAPI1*)api)->RaiseEvent = Events::ADDONAPI_RaiseEvent;
			((AddonAPI1*)api)->SubscribeEvent = Events::Subscribe;
			((AddonAPI1*)api)->UnsubscribeEvent = Events::Unsubscribe;

			((AddonAPI1*)api)->RegisterWndProc = WndProc::Register;
			((AddonAPI1*)api)->DeregisterWndProc = WndProc::Deregister;

			((AddonAPI1*)api)->RegisterKeybindWithString = Keybinds::ADDONAPI_RegisterWithString;
			((AddonAPI1*)api)->RegisterKeybindWithStruct = Keybinds::ADDONAPI_RegisterWithStruct;
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

			((AddonAPI2*)api)->Log = ADDONAPI_LogMessage2;

			((AddonAPI2*)api)->RaiseEvent = Events::ADDONAPI_RaiseEvent;
			((AddonAPI2*)api)->RaiseEventNotification = Events::ADDONAPI_RaiseNotification;
			((AddonAPI2*)api)->SubscribeEvent = Events::Subscribe;
			((AddonAPI2*)api)->UnsubscribeEvent = Events::Unsubscribe;

			((AddonAPI2*)api)->RegisterWndProc = WndProc::Register;
			((AddonAPI2*)api)->DeregisterWndProc = WndProc::Deregister;
			((AddonAPI2*)api)->SendWndProcToGameOnly = WndProc::SendWndProcToGame;

			((AddonAPI2*)api)->RegisterKeybindWithString = Keybinds::ADDONAPI_RegisterWithString;
			((AddonAPI2*)api)->RegisterKeybindWithStruct = Keybinds::ADDONAPI_RegisterWithStruct;
			((AddonAPI2*)api)->DeregisterKeybind = Keybinds::Deregister;

			((AddonAPI2*)api)->GetResource = DataLink::GetResource;
			((AddonAPI2*)api)->ShareResource = DataLink::ShareResource;

			((AddonAPI2*)api)->GetTexture = TextureLoader::Get;
			((AddonAPI2*)api)->GetTextureOrCreateFromFile = TextureLoader::ADDONAPI_GetOrCreateFromFile;
			((AddonAPI2*)api)->GetTextureOrCreateFromResource = TextureLoader::ADDONAPI_GetOrCreateFromResource;
			((AddonAPI2*)api)->GetTextureOrCreateFromURL = TextureLoader::ADDONAPI_GetOrCreateFromURL;
			((AddonAPI2*)api)->GetTextureOrCreateFromMemory = TextureLoader::ADDONAPI_GetOrCreateFromMemory;
			((AddonAPI2*)api)->LoadTextureFromFile = TextureLoader::LoadFromFile;
			((AddonAPI2*)api)->LoadTextureFromResource = TextureLoader::LoadFromResource;
			((AddonAPI2*)api)->LoadTextureFromURL = TextureLoader::LoadFromURL;
			((AddonAPI2*)api)->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

			((AddonAPI2*)api)->AddShortcut = GUI::QuickAccess::AddShortcut;
			((AddonAPI2*)api)->RemoveShortcut = GUI::QuickAccess::RemoveShortcut;
			((AddonAPI2*)api)->NotifyShortcut = GUI::QuickAccess::NotifyShortcut;
			((AddonAPI2*)api)->AddSimpleShortcut = GUI::QuickAccess::AddSimpleShortcut;
			((AddonAPI2*)api)->RemoveSimpleShortcut = GUI::QuickAccess::RemoveSimpleShortcut;

			((AddonAPI2*)api)->Translate = Localization::ADDONAPI_Translate;
			((AddonAPI2*)api)->TranslateTo = Localization::ADDONAPI_TranslateTo;

			ApiDefs.insert({ aVersion, api });
			return api;

		case 3:
			api = new AddonAPI3();

			((AddonAPI3*)api)->SwapChain = Renderer::SwapChain;
			((AddonAPI3*)api)->ImguiContext = Renderer::GuiContext;
			((AddonAPI3*)api)->ImguiMalloc = ImGui::MemAlloc;
			((AddonAPI3*)api)->ImguiFree = ImGui::MemFree;
			((AddonAPI3*)api)->RegisterRender = GUI::Register;
			((AddonAPI3*)api)->DeregisterRender = GUI::Deregister;

			((AddonAPI3*)api)->GetGameDirectory = Path::GetGameDirectory;
			((AddonAPI3*)api)->GetAddonDirectory = Path::GetAddonDirectory;
			((AddonAPI3*)api)->GetCommonDirectory = Path::GetCommonDirectory;

			((AddonAPI3*)api)->CreateHook = MH_CreateHook;
			((AddonAPI3*)api)->RemoveHook = MH_RemoveHook;
			((AddonAPI3*)api)->EnableHook = MH_EnableHook;
			((AddonAPI3*)api)->DisableHook = MH_DisableHook;

			((AddonAPI3*)api)->Log = ADDONAPI_LogMessage2;

			((AddonAPI3*)api)->SendAlert = GUI::Alerts::Notify;

			((AddonAPI3*)api)->RaiseEvent = Events::ADDONAPI_RaiseEvent;
			((AddonAPI3*)api)->RaiseEventNotification = Events::ADDONAPI_RaiseNotification;
			((AddonAPI3*)api)->RaiseEventTargeted = Events::ADDONAPI_RaiseEventTargeted;
			((AddonAPI3*)api)->RaiseEventNotificationTargeted = Events::ADDONAPI_RaiseNotificationTargeted;
			((AddonAPI3*)api)->SubscribeEvent = Events::Subscribe;
			((AddonAPI3*)api)->UnsubscribeEvent = Events::Unsubscribe;

			((AddonAPI3*)api)->RegisterWndProc = WndProc::Register;
			((AddonAPI3*)api)->DeregisterWndProc = WndProc::Deregister;
			((AddonAPI3*)api)->SendWndProcToGameOnly = WndProc::SendWndProcToGame;

			((AddonAPI3*)api)->RegisterKeybindWithString = Keybinds::ADDONAPI_RegisterWithString;
			((AddonAPI3*)api)->RegisterKeybindWithStruct = Keybinds::ADDONAPI_RegisterWithStruct;
			((AddonAPI3*)api)->DeregisterKeybind = Keybinds::Deregister;

			((AddonAPI3*)api)->GetResource = DataLink::GetResource;
			((AddonAPI3*)api)->ShareResource = DataLink::ShareResource;

			((AddonAPI3*)api)->GetTexture = TextureLoader::Get;
			((AddonAPI3*)api)->GetTextureOrCreateFromFile = TextureLoader::ADDONAPI_GetOrCreateFromFile;
			((AddonAPI3*)api)->GetTextureOrCreateFromResource = TextureLoader::ADDONAPI_GetOrCreateFromResource;
			((AddonAPI3*)api)->GetTextureOrCreateFromURL = TextureLoader::ADDONAPI_GetOrCreateFromURL;
			((AddonAPI3*)api)->GetTextureOrCreateFromMemory = TextureLoader::ADDONAPI_GetOrCreateFromMemory;
			((AddonAPI3*)api)->LoadTextureFromFile = TextureLoader::LoadFromFile;
			((AddonAPI3*)api)->LoadTextureFromResource = TextureLoader::LoadFromResource;
			((AddonAPI3*)api)->LoadTextureFromURL = TextureLoader::LoadFromURL;
			((AddonAPI3*)api)->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

			((AddonAPI3*)api)->AddShortcut = GUI::QuickAccess::AddShortcut;
			((AddonAPI3*)api)->RemoveShortcut = GUI::QuickAccess::RemoveShortcut;
			((AddonAPI3*)api)->NotifyShortcut = GUI::QuickAccess::NotifyShortcut;
			((AddonAPI3*)api)->AddSimpleShortcut = GUI::QuickAccess::AddSimpleShortcut;
			((AddonAPI3*)api)->RemoveSimpleShortcut = GUI::QuickAccess::RemoveSimpleShortcut;

			((AddonAPI3*)api)->Translate = Localization::ADDONAPI_Translate;
			((AddonAPI3*)api)->TranslateTo = Localization::ADDONAPI_TranslateTo;

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
		case 3:
			return sizeof(AddonAPI3);
		}

		return 0;
	}

	std::string GetOwner(void* aAddress)
	{
		if (aAddress == nullptr)
		{
			return "(null)";
		}

		//const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (auto& addon : Addons)
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

	Addon* FindAddonBySig(signed int aSignature)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aSignature](Addon* addon) {
			// check if it has definitions and if the signature is the same
			return addon->Definitions && addon->Definitions->Signature == aSignature;
			});

		if (it != Addons.end()) {
			return *it;
		}

		return nullptr;
	}
	Addon* FindAddonByPath(const std::filesystem::path& aPath)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aPath](Addon* addon) {
			// check if it has definitions and if the signature is the same
			return addon->Path == aPath;
			});

		if (it != Addons.end()) {
			return *it;
		}

		return nullptr;
	}
	Addon* FindAddonByMatchSig(signed int aMatchSignature)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aMatchSignature](Addon* addon) {
			// check if it has definitions and if the signature is the same
			return addon->MatchSignature == aMatchSignature;
			});

		if (it != Addons.end()) {
			return *it;
		}

		return nullptr;
	}
	Addon* FindAddonByMD5(std::vector<unsigned char> aMD5)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aMD5](Addon* addon) {
			// check if it has definitions and if the signature is the same
			return addon->MD5 == aMD5;
			});

		if (it != Addons.end()) {
			return *it;
		}

		return nullptr;
	}

	void SortAddons()
	{
		std::sort(Addons.begin(), Addons.end(), [](Addon* lhs, Addon* rhs)
			{
				if (lhs->Definitions && rhs->Definitions)
				{
					std::string lname = String::Normalize(lhs->Definitions->Name);
					std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
					std::string rname = String::Normalize(rhs->Definitions->Name);
					std::transform(rname.begin(), rname.end(), rname.begin(), ::tolower);

					return lname < rname;
				}

				std::string lpath = lhs->Path.string();
				std::transform(lpath.begin(), lpath.end(), lpath.begin(), ::tolower);
				std::string rpath = rhs->Path.string();
				std::transform(rpath.begin(), rpath.end(), rpath.begin(), ::tolower);

				return lpath < rpath;
			});
	}

	void GetGameBuild()
	{
		/* prepare client request */
		httplib::Client client("http://assetcdn.101.arenanetworks.com");
		client.enable_server_certificate_verification(false);

		std::string buildStr;

		size_t bytesWritten = 0;
		auto result = client.Get("/latest64/101", [&](const char* data, size_t data_length) {
			buildStr += data;
			bytesWritten += data_length;
			return true;
			});

		if (!result || result->status != 200)
		{
			LogWarning(CH_LOADER, "Error fetching \"http://assetcdn.101.arenanetworks.com/latest64/101\"");
			return;
		}

		int gameBuild = std::stoi(buildStr);

		int lastGameBuild = 0;

		if (!Settings::Settings.is_null())
		{
			if (!Settings::Settings[OPT_LASTGAMEBUILD].is_null())
			{
				Settings::Settings[OPT_LASTGAMEBUILD].get_to(lastGameBuild);
			}
		}

		if (gameBuild - lastGameBuild > 350 && lastGameBuild != 0)
		{
			DisableVolatileUntilUpdate = true;
			LogWarning(CH_LOADER, "Game updated. Current Build %d. Old Build: %d. Disabling volatile addons until they update.", gameBuild, lastGameBuild);

			GUI::Alerts::Notify(String::Format("%s\n%s", Language.Translate("((000001))"), Language.Translate("((000002))")).c_str());
		}

		Settings::Settings[OPT_LASTGAMEBUILD] = gameBuild;
		Settings::Save();
	}
}
