///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.cpp
/// Description  :  Handles addon hot-loading, updates etc.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Loader.h"

#include <chrono>
#include <Psapi.h>
#include <regex>
#include <vector>
#include <Windows.h>

#include "Context.h"
#include "Consts.h"
#include "Engine/Index/Index.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"

#include "AddonDefinition.h"
#include "FuncDefs.h"

#include "Engine/Events/EvtApi.h"
#include "UI/UiContext.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/Services/QoL/EscapeClosing.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"
#include "imgui/imgui.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "minhook/mh_hook.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Localization/Localization.h"
#include "Engine/Logging/LogApi.h"
#include "GW2/Mumble/Reader.h"
#include "Engine/Settings/Settings.h"
#include "Engine/Textures/TxLoader.h"
#include "Engine/Updater/Updater.h"
#include "API/ApiFunctionMapper.h"

#include "Util/DLL.h"
#include "Util/MD5.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"

#include "ArcDPS.h"
#include "Library.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "httplib/httplib.h"

#define LOADER_WAITTIME_MS 100
#define EXT_DLL ".dll"
#define EXT_UPDATE ".update"
#define EXT_OLD ".old"
#define EXT_UNINSTALL ".uninstall"

namespace Loader
{
	NexusLinkData_t*          NexusLink = nullptr;
	Mumble::Identity*       MumbleIdentity = nullptr;

	std::mutex              Mutex;
	std::unordered_map<
		std::filesystem::path,
		ELoaderAction
	>                       QueuedAddons;
	std::vector<Addon_t*>     Addons;
	bool                    HasCustomConfig;
	std::filesystem::path   ConfigPath;
	std::vector<signed int> RequestedAddons;

	int                     DirectoryChangeCountdown = 0;
	std::condition_variable ConVar;
	std::mutex              ThreadMutex;
	std::thread             LoaderThread;
	bool                    IsSuspended = false;

	PIDLIST_ABSOLUTE        FSItemList;
	ULONG                   FSNotifierID;

	std::vector<signed int> WhitelistedAddons;				/* List of addons that should be loaded on initial startup. */

	bool                    DisableVolatileUntilUpdate = false;

	CDataLinkApi*  DataLink = nullptr;
	CLogApi*       Logger   = nullptr;
	CEventApi*     EventApi = nullptr;
	CUpdater*      Updater  = nullptr;
	CAlerts*       Alerts   = nullptr;
	CLocalization* Language = nullptr;

	void Initialize()
	{
		CContext* ctx = CContext::GetContext();
		DataLink = ctx->GetDataLink();
		Logger = ctx->GetLogger();
		EventApi = ctx->GetEventApi();
		Updater = ctx->GetUpdater();
		CUiContext* uictx = ctx->GetUIContext();
		Alerts = uictx->GetAlerts();
		Language = ctx->GetLocalization();

		NexusLink = (NexusLinkData_t*)DataLink->ShareResource(DL_NEXUS_LINK, sizeof(NexusLinkData_t), "", true);
		MumbleIdentity = (Mumble::Identity*)DataLink->ShareResource(DL_MUMBLE_LINK_IDENTITY, sizeof(Mumble::Identity), "", false);

		ConfigPath = Index(EPath::AddonConfigDefault);

		if (CmdLine::HasArgument("-ggaddons"))
		{
			std::vector<std::string> idList = String::Split(CmdLine::GetArgumentValue("-ggaddons"), ",");

			/* if -ggaddons param is config path, else it's id list*/
			if (idList.size() == 1 && String::Contains(idList[0], ".json"))
			{
				/* overwrite index path */
				ConfigPath = idList[0];
			}
			else
			{
				HasCustomConfig = true;

				for (std::string addonId : idList)
				{
					try
					{
						signed int i = std::stoi(addonId);
						RequestedAddons.push_back(i);
					}
					catch (const std::invalid_argument& e)
					{
						Logger->Trace(CH_CORE, "Invalid argument (-ggaddons): %s (exc: %s)", addonId.c_str(), e.what());
					}
					catch (const std::out_of_range& e)
					{
						Logger->Trace(CH_CORE, "Out of range (-ggaddons): %s (exc: %s)", addonId.c_str(), e.what());
					}
				}
			}
		}
		
		if (State::Nexus == ENexusState::LOADED)
		{
			if (Index(EPath::AddonConfigDefault) != ConfigPath)
			{
				Logger->Info(CH_LOADER, "AddonConfig path specified. Saving to \"%s\"", ConfigPath.string().c_str());
			}

			LoadAddonConfig();

			//FSItemList = ILCreateFromPathA(Index::GetAddonDirectory(nullptr));
			std::wstring addonDirW = String::ToWString(Index(EPath::DIR_ADDONS).string());
			HRESULT hresult = SHParseDisplayName(
				addonDirW.c_str(),
				0,
				&FSItemList,
				0xFFFFFFFF,
				0
			);
			if (FSItemList == 0)
			{
				Logger->Critical(CH_LOADER, "Loader disabled. Reason: SHParseDisplayName(Index::D_GW2_ADDONS) returned %d.", hresult);
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
				Logger->Critical(CH_LOADER, "Loader disabled. Reason: SHChangeNotifyRegister(...) returned 0.");
				return;
			}

			std::thread(Library::Fetch).detach();
			std::thread(ArcDPS::GetPluginLibrary).detach();

			std::thread checkLaunchSequence([]()
			{
				Sleep(5000);
				int nothingCounter = 0;
				while (State::Nexus < ENexusState::SHUTTING_DOWN && !NexusLink->IsGameplay)
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

			while (Addons.size() != 0)
			{
				UnloadAddon(Addons.front()->Path);

				/* sanity check in case UnloadAddon removes the entry, because the file is no longer on disk. */
				if (Addons.size() != 0)
				{
					Addons.erase(Addons.begin());
				}
			}
		}
	}

	void LoadAddonConfig()
	{
		if (std::filesystem::exists(ConfigPath))
		{
			try
			{
				std::ifstream file(ConfigPath);

				json cfg = json::parse(file);
				for (json addonInfo : cfg)
				{
					signed int signature = 0;
					if (!addonInfo["Signature"].is_null()) { addonInfo["Signature"].get_to(signature); }

					if (signature == 0) { continue; }

					Addon_t* addon = FindAddonBySig(signature);

					if (!addon)
					{
						addon = FindAddonByMatchSig(signature);
					}

					if (!addon)
					{
						addon = new Addon_t{};
						addon->State = EAddonState::None;
						Addons.push_back(addon);
					}

					if (!addonInfo["IsPausingUpdates"].is_null()) { addonInfo["IsPausingUpdates"].get_to(addon->IsPausingUpdates); }
					if (!addonInfo["IsDisabledUntilUpdate"].is_null()) { addonInfo["IsDisabledUntilUpdate"].get_to(addon->IsDisabledUntilUpdate); }
					if (!addonInfo["AllowPrereleases"].is_null()) { addonInfo["AllowPrereleases"].get_to(addon->AllowPrereleases); }
					if (!addonInfo["IsFavorite"].is_null()) { addonInfo["IsFavorite"].get_to(addon->IsFavorite); }

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
				Logger->Warning(CH_LOADER, "AddonConfig.json could not be parsed. Error: %s", ex.what());
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
				{"Name", addon->Definitions ? addon->Definitions->Name : "-"},
				{"Signature", sig},
				{"IsLoaded", addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED || addon->IsFlaggedForEnable},
				{"IsPausingUpdates", addon->IsPausingUpdates},
				{"IsDisabledUntilUpdate", addon->IsDisabledUntilUpdate},
				{"AllowPrereleases", addon->AllowPrereleases},
				{"IsFavorite", addon->IsFavorite}
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

		std::ofstream file(ConfigPath);
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
					if (SHGetPathFromIDListA(ppidl[0], path))
					{
						if (Index(EPath::DIR_ADDONS) == std::string(path))
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
			Logger->Trace(CH_LOADER, "Processing changes after waiting for %ums.", time / std::chrono::milliseconds(1));

			const std::lock_guard<std::mutex> lock(Mutex);
			// check all tracked addons
			for (Addon_t* addon : Addons)
			{
				// if addon no longer on disk (also check if the path is not null, else it's from config)
				if (!addon->Path.empty() && !std::filesystem::exists(addon->Path))
				{
					QueueAddon(ELoaderAction::Unload, addon->Path);
					continue;
				}

				// get md5 of each file currently on disk and compare to tracked md5
				// also check if an update is available (e.g. "addon.dll" + ".update" -> "addon.dll.update" exists)
				std::vector<unsigned char> md5 = MD5Util::FromFile(addon->Path);
				std::filesystem::path updatePath = addon->Path.string() + EXT_UPDATE;
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
			for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index(EPath::DIR_ADDONS)))
			{
				std::filesystem::path path = entry.path();

				if (std::filesystem::is_directory(path))
				{
					continue;
				}

				// if already tracked
				Addon_t* exists = FindAddonByPath(path);
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

				if (path.extension() == EXT_DLL)
				{
					std::vector<unsigned char> md5 = MD5Util::FromFile(path);
					if (FindAddonByMD5(md5) == nullptr)
					{
						QueueAddon(ELoaderAction::Load, path);
					}
				}

				if (path.extension() == EXT_UNINSTALL)
				{
					try
					{
						std::filesystem::remove(path);
					}
					catch (std::filesystem::filesystem_error fErr)
					{
						Logger->Debug(CH_LOADER, "%s", fErr.what());
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

		Addon_t* addon = FindAddonByPath(aPath);

		if (addon)
		{
			if (addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED)
			{
				//Logger->Warning(CH_LOADER, "Cancelled loading \"%s\". Already loaded.", strFile.c_str());
				return;
			}
		}
		else
		{
			allocNew = true;
			addon = new Addon_t{};
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
			const std::error_condition ecnd = std::system_category().default_error_condition(GetLastError());
			Logger->Warning(CH_LOADER, "Failed LoadLibrary on \"%s\". Incompatible.\nError Code %u : %s", strFile.c_str(), ecnd.value(), ecnd.message().c_str());
			addon->State = EAddonState::NotLoadedIncompatible;

			if (allocNew)
			{
				Addons.push_back(addon); // track this anyway
			}

			return;
		}

		/* doesn't have GetAddonDef */
		if (DLL::FindFunction(addon->Module, &getAddonDef, "GetAddonDef") == false)
		{
			Logger->Warning(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Incompatible.", strFile.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;

			if (allocNew)
			{
				Addons.push_back(addon); // track this anyway
			}

			return;
		}

		AddonDef_t* tmpDefs = getAddonDef();

		/* addon defs are nullptr */
		if (tmpDefs == nullptr)
		{
			Logger->Warning(CH_LOADER, "\"%s\" is exporting \"GetAddonDef\" but returned a nullptr. Incompatible.", strFile.c_str());
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
		AddonDef_t::Free(&addon->Definitions);
		AddonDef_t::Copy(tmpDefs, &addon->Definitions);

		/* get stored info about addon and apply to addon */
		if (allocNew)
		{
			Addon_t* stored = FindAddonByMatchSig(addon->Definitions->Signature);

			// stored exists and is "unclaimed"
			if (stored && stored->State == EAddonState::None)
			{
				/* we have some settings/info stored, we merge and delete the new alloc */
				Addon_t* alloc = addon;
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

		/* doesn't fulfill min reqs */
		if (!addon->Definitions->HasMinimumRequirements())
		{
			std::string reqMsg = String::Format("\"%s\" does not fulfill minimum requirements. Incompatible. Reasons: \n", strFile.c_str());
			if (!addon->Definitions->Signature) { reqMsg.append("Signature is 0.\n"); }
			if (!addon->Definitions->Name) { reqMsg.append("Name is nullptr.\n"); }
			if (!addon->Definitions->Author) { reqMsg.append("Author is nullptr.\n"); }
			if (!addon->Definitions->Description) { reqMsg.append("Description is nullptr.\n"); }
			if (!addon->Definitions->Load) { reqMsg.append("Load function is nullptr.\n"); }
			if (!addon->Definitions->Unload &&
				!addon->Definitions->HasFlag(EAddonFlags::DisableHotloading))
			{
				reqMsg.append("Unload function is nullptr, but Hotloading was not disabled.");
			}
			Logger->Warning(CH_LOADER, reqMsg.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}

		/* check if duplicate signature */
		auto duplicate = std::find_if(Addons.begin(), Addons.end(), [addon](Addon_t* cmpAddon) {
			// check if it has definitions and if the signature is the same
			return	cmpAddon->Path != addon->Path &&
				cmpAddon->Definitions &&
				cmpAddon->Definitions->Signature == addon->Definitions->Signature;
		});
		if (duplicate != Addons.end()) {
			Logger->Warning(CH_LOADER, "\"%s\" or another addon with this signature (%d) is already loaded. Duplicate.", strFile.c_str(), addon->Definitions->Signature);
			FreeLibrary(addon->Module);
			AddonDef_t::Free(&addon->Definitions);
			addon->State = EAddonState::NotLoadedDuplicate;
			addon->Module = nullptr;
			return;
		}

		/* fix update provider in case none was set, but a link was provided. */
		if (addon->Definitions->Provider == EUpdateProvider::None && addon->Definitions->UpdateLink)
		{
			addon->Definitions->Provider = GetProvider(addon->Definitions->UpdateLink);

			Logger->Info(CH_LOADER, "\"%s\" does not have a provider set, but declares an update resource. Deduced Provider %d from URL.", addon->Definitions->Name, addon->Definitions->Provider);
		}

		bool isInitialLoad = addon->State == EAddonState::None;

		// if not on whitelist and its the initial load (aka not manually invoked)
		auto it = std::find(WhitelistedAddons.begin(), WhitelistedAddons.end(), addon->Definitions->Signature);
		bool shouldLoad = it != WhitelistedAddons.end() || !isInitialLoad;

		// if pausing updates, but wasn't set to be disabled until update
		bool shouldCheckForUpdate = !(addon->IsPausingUpdates && !addon->IsDisabledUntilUpdate);

		/* set DUU state if game has updated and addon is volatile and this is the intial load */
		if (isInitialLoad && addon->Definitions->HasFlag(EAddonFlags::IsVolatile) && DisableVolatileUntilUpdate && !addon->IsPausingUpdates)
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

				AddonInfo_t addonInfo
				{
					addon->Definitions->Signature,
					addon->Definitions->Name,
					addon->Definitions->Version,
					addon->Definitions->Provider,
					addon->Definitions->UpdateLink != nullptr
						? addon->Definitions->UpdateLink
						: "",
					addon->MD5,
					addon->AllowPrereleases
				};

				if (addon->Definitions->Provider != EUpdateProvider::Self && Updater->UpdateAddon(tmpPath, addonInfo))
				{
					Logger->Info(CH_LOADER, "Update available for \"%s\".", tmpPath.string().c_str());
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
							Alerts->Notify(EAlertType::Info, String::Format("%s %s", addon->Definitions->Name, Language->Translate("((000079))")).c_str());
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
			//Logger->Info(CH_LOADER, "\"%s\" was not requested via start parameter or last state was disabled. Skipped.", strFile.c_str(), addon->Definitions->Signature);
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoaded;
			addon->Module = nullptr;
			return;
		}

		if (addon->IsDisabledUntilUpdate && DisableVolatileUntilUpdate)
		{
			// show message that addon was disabled due to game update
			EventApi->Raise(EV_VOLATILE_ADDON_DISABLED, &addon->Definitions->Signature);
			static bool didNotify = false;
			if (!didNotify)
			{
				Alerts->Notify(EAlertType::Info, String::Format("%s\n%s", Language->Translate("((000001))"), Language->Translate("((000002))")).c_str());
				didNotify = true;
			}
			Alerts->Notify(EAlertType::Info, String::Format("%s %s", addon->Definitions->Name, Language->Translate("((000073))")).c_str());

			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoaded;
			addon->Module = nullptr;
			return;
		}

		/* (effectively duplicate check) if someone wants to do shenanigans and inject a different integration module */
		if (addon->Definitions->Signature == 0xFED81763 && aPath != Index(EPath::ArcdpsIntegration))
		{
			Logger->Warning(CH_LOADER, "\"%s\" declares signature 0xFED81763 but is not the actual Nexus ArcDPS Integration. Either this was in error or an attempt to tamper with Nexus files. Incompatible.", strFile.c_str());
			FreeLibrary(addon->Module);
			addon->State = EAddonState::NotLoadedIncompatible;
			addon->Module = nullptr;
			return;
		}

		AddonAPI_t* api = ADDONAPI::Get(addon->Definitions->APIVersion); // will be nullptr if doesn't exist or APIVersion = 0

		// if the api doesn't exist and there was one requested
		if (api == nullptr && addon->Definitions->APIVersion != 0)
		{
			Logger->Warning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Incompatible.", strFile.c_str(), addon->Definitions->APIVersion);
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

		EventApi->Raise(EV_ADDON_LOADED, &addon->Definitions->Signature);
		EventApi->Raise(EV_MUMBLE_IDENTITY_UPDATED, MumbleIdentity);

		addon->State = locked ? EAddonState::LoadedLOCKED : EAddonState::Loaded;
		SaveAddonConfig();

		Logger->Info(CH_LOADER, u8"Loaded addon: %s\nSignature: %d\nAddress Space: %p - %p\nAPI Version: %d\nTook %uµs to load.",
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

	void CallUnloadAndVerify(const std::filesystem::path& aPath, Addon_t* aAddon)
	{
		bool isShutdown = State::Nexus == ENexusState::SHUTTING_DOWN;

		if (!isShutdown)
		{
			/* cache the flag, save that the addon will disable, then restore it */
			bool flagDisable = aAddon->IsFlaggedForDisable;
			aAddon->IsFlaggedForDisable = true;

			const std::lock_guard<std::mutex> lock(Mutex);
			SaveAddonConfig();

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

			CContext* ctx = CContext::GetContext();
			CUiContext* uictx = ctx->GetUIContext();

			int evRefs = ctx->GetEventApi()->Verify(startAddress, endAddress);
			int uiRefs = uictx->Verify(startAddress, endAddress);
			int fnRefs = uictx->GetFontManager()->Verify(startAddress, endAddress);
			int esRefs = uictx->GetEscapeClosingService()->Verify(startAddress, endAddress);
			int qaRefs = uictx->GetQuickAccess()->Verify(startAddress, endAddress);
			int kbRefs = ctx->GetInputBindApi()->Verify(startAddress, endAddress);
			int riRefs = ctx->GetRawInputApi()->Verify(startAddress, endAddress);
			int txRefs = ctx->GetTextureService()->Verify(startAddress, endAddress);
			int leftoverRefs = evRefs + uiRefs + qaRefs + kbRefs + riRefs + txRefs;

			if (leftoverRefs > 0)
			{
				std::string str = String::Format("Removed %d unreleased references from \"%s\".", leftoverRefs, aPath.filename().string().c_str());
				str.append(" ");
				str.append("Make sure your addon releases all references during Addon::Unload().\n");
				if (evRefs) { str.append(String::Format("Events: %d\n", evRefs)); }
				if (uiRefs) { str.append(String::Format("UI: %d\n", uiRefs)); }
				if (fnRefs) { str.append(String::Format("Fonts: %d\n", fnRefs)); }
				if (esRefs) { str.append(String::Format("CloseOnEscape: %d\n", esRefs)); }
				if (qaRefs) { str.append(String::Format("QuickAccess: %d\n", qaRefs)); }
				if (kbRefs) { str.append(String::Format("InputBinds: %d\n", kbRefs)); }
				if (riRefs) { str.append(String::Format("WndProc: %d\n", riRefs)); }
				if (txRefs) { str.append(String::Format("Textures: %d", txRefs)); }
				Logger->Warning(CH_LOADER, str.c_str());
			}
		}

		aAddon->IsWaitingForUnload = false;
		Logger->Info(CH_LOADER, u8"Unloaded addon: %s (Took %uµs.)", aPath.filename().string().c_str(), time / std::chrono::microseconds(1));
	}

	void UnloadAddon(const std::filesystem::path& aPath, bool aDoReload)
	{
		std::string path = aPath.string();
		std::string strFile = aPath.filename().string();

		Addon_t* addon = FindAddonByPath(aPath);

		bool isShutdown = State::Nexus == ENexusState::SHUTTING_DOWN;

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
						AddonDef_t::Free(&addon->Definitions);
					}

					/* remove addon from list, it's no longer on disk*/
					if (!isShutdown)
					{
						Addons.erase(it);
					}
				}
			}
			//Logger->Warning(CH_LOADER, "Cancelled unload of \"%s\". EAddonState = %d.", strFile.c_str(), addon->State);
			return;
		}

		if (addon->Definitions)
		{
			// Either normal unload
			// or shutting down anyway, addon has Unload defined, so it can save settings etc
			if ((addon->State == EAddonState::Loaded) || (addon->State == EAddonState::LoadedLOCKED && isShutdown))
			{
				std::thread unloadTask([addon, aPath, aDoReload, isShutdown]()
				{
					CallUnloadAndVerify(aPath, addon);

					if (!isShutdown)
					{
						EventApi->Raise(EV_ADDON_UNLOADED, &addon->Definitions->Signature);

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

	void FreeAddon(const std::filesystem::path& aPath)
	{
		std::string path = aPath.string();
		std::string strFile = aPath.filename().string();

		Addon_t* addon = FindAddonByPath(aPath);

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
					AddonDef_t::Free(&addon->Definitions);
				}
				Addons.erase(it);
			}
		}

		//Logger->Debug(CH_LOADER, "Called FreeLibrary() %d times on \"%s\".", freeCalls, strFile.c_str());

		Loader::NotifyChanges();
	}

	void UninstallAddon(const std::filesystem::path& aPath)
	{
		/* check both LoadedLOCKED, but also Loaded as a sanity check */
		Addon_t* addon = FindAddonByPath(aPath);

		/* if it's still loaded due to being locked (or for some obscure other reason)
		try to move addon.dll to addon.dll.uninstall, so it will be deleted on next restart */
		if (addon)
		{
			if (addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED || addon->IsWaitingForUnload)
			{
				try
				{
					std::filesystem::rename(aPath, aPath.string() + EXT_UNINSTALL);
					addon->IsFlaggedForUninstall = true;
					Logger->Warning(CH_LOADER, "Addon is stilled loaded, it will be uninstalled the next time the game is restarted: %s", aPath.string().c_str());
				}
				catch (std::filesystem::filesystem_error fErr)
				{
					Logger->Debug(CH_LOADER, "%s", fErr.what());
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
						AddonDef_t::Free(&addon->Definitions);
					}
					Addons.erase(it);
				}
				Logger->Info(CH_LOADER, "Uninstalled addon: %s", aPath.string().c_str());
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				Logger->Debug(CH_LOADER, "%s", fErr.what());
				return;
			}
		}
	}

	bool UpdateSwapAddon(const std::filesystem::path& aPath)
	{
		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + EXT_OLD;
		std::filesystem::path pathUpdate = aPath.string() + EXT_UPDATE;

		/* remove leftover .old */
		if (std::filesystem::exists(pathOld))
		{
			try
			{
				std::filesystem::remove(pathOld);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				Logger->Debug(CH_LOADER, "%s", fErr.what());
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
				Logger->Debug(CH_LOADER, "%s", fErr.what());
				return false;
			}
		}

		return false;
	}

	std::string GetOwner(void* aAddress)
	{
		if (aAddress == nullptr)
		{
			return NULLSTR;
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
					return addon->Definitions ? addon->Definitions->Name : NULLSTR;
				}
			}

			CContext* ctx = CContext::GetContext();

			void* startAddress = ctx->GetModule();
			void* endAddress = ((PBYTE)ctx->GetModule()) + ctx->GetModuleSize();

			if (aAddress >= startAddress && aAddress <= endAddress)
			{
				return "Nexus";
			}
		}

		return NULLSTR;
	}

	Addon_t* FindAddonBySig(signed int aSignature)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aSignature](Addon_t* addon) { return addon->Definitions && addon->Definitions->Signature == aSignature; });

		if (it != Addons.end())
		{
			return *it;
		}

		return nullptr;
	}
	Addon_t* FindAddonByPath(const std::filesystem::path& aPath)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aPath](Addon_t* addon) { return addon->Path == aPath; });

		if (it != Addons.end())
		{
			return *it;
		}

		return nullptr;
	}
	Addon_t* FindAddonByMatchSig(signed int aMatchSignature)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aMatchSignature](Addon_t* addon) { return addon->MatchSignature == aMatchSignature; });

		if (it != Addons.end())
		{
			return *it;
		}

		return nullptr;
	}
	Addon_t* FindAddonByMD5(std::vector<unsigned char> aMD5)
	{
		auto it = std::find_if(Addons.begin(), Addons.end(), [aMD5](Addon_t* addon) { return addon->MD5 == aMD5; });

		if (it != Addons.end())
		{
			return *it;
		}

		return nullptr;
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
			Logger->Warning(CH_LOADER, "Error fetching \"http://assetcdn.101.arenanetworks.com/latest64/101\"\nError: %s", httplib::to_string(result.error()).c_str());
			return;
		}

		int gameBuild = std::stoi(buildStr);

		CContext* ctx = CContext::GetContext();
		CSettings* settingsCtx = ctx->GetSettingsCtx();

		int lastGameBuild = settingsCtx->Get<int>(OPT_LASTGAMEBUILD, 0);

		if (gameBuild - lastGameBuild > 350 && lastGameBuild != 0)
		{
			DisableVolatileUntilUpdate = true;
			Logger->Warning(CH_LOADER, "Game updated. Current Build %d. Old Build: %d. Disabling volatile addons until they update.", gameBuild, lastGameBuild);
		}

		if (lastGameBuild != gameBuild)
		{
			settingsCtx->Set(OPT_LASTGAMEBUILD, gameBuild);
		}
	}
}
