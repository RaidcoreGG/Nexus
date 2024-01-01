#include "Loader.h"

#include <set>
#include <Windows.h>
#include <Psapi.h>
#include <regex>
#include <malloc.h>
#include <vector>

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

#include "httpslib.h"

#define LOADER_WAITTIME 1

namespace Loader
{
	std::mutex Mutex;
	std::unordered_map<std::filesystem::path, ELoaderAction> QueuedAddons;
	std::unordered_map<std::filesystem::path, Addon*> Addons;
	std::map<int, AddonAPI*> ApiDefs;

	int DirectoryChangeCountdown = 0;
	std::thread LoaderThread;

	PIDLIST_ABSOLUTE FSItemList;
	ULONG FSNotifierID;

	std::string dll = ".dll";
	std::string dllUpdate = ".update";

	void Initialize()
	{
		if (State::Nexus == ENexusState::LOADED)
		{
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
				SHCNE_UPDATEITEM,
				WM_ADDONDIRUPDATE,
				1,
				&changeentry
			);

			if (FSNotifierID <= 0)
			{
				LogCritical(CH_LOADER, "Loader disabled. Reason: SHChangeNotifyRegister(...) returned 0.");
				return;
			}

			NotifyChanges(); // Invoke once because addon dir doesn't change on init
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
						LogWarning(CH_LOADER, "Despite being unloaded \"%s\" still has an HMODULE defined, calling FreeLibrary().", Addons.begin()->first.string().c_str());
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
				UnloadAddon(it->first);
				LoadAddon(it->first);
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

	void NotifyChanges()
	{
		if (DirectoryChangeCountdown == 0)
		{
			LoaderThread = std::thread(AwaitChanges);
			LoaderThread.detach();
		}

		DirectoryChangeCountdown = LOADER_WAITTIME;
	}
	void AwaitChanges()
	{
		while (DirectoryChangeCountdown > 0)
		{
			Sleep(1000);
			DirectoryChangeCountdown--;
		}

		ProcessChanges();
	}
	void ProcessChanges()
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		// check all tracked addons
		for (auto& it : Addons)
		{
			// if addon no longer on disk
			if (!std::filesystem::exists(it.first))
			{
				LogDebug(CH_LOADER, "%s no longer exists. Attempting to unload.", it.first.string().c_str());
				UnloadAddon(it.first);
				Addons.erase(it.first); // sanity check
				continue;
			}

			std::vector<unsigned char> md5 = MD5FromFile(it.first);

			// if addon changed
			if (it.second->MD5.empty() || it.second->MD5 != md5)
			{
				//QueueAddon(ELoaderAction::Reload, it.first);
				UnloadAddon(it.first);
				LoadAddon(it.first);
			}
		}

		// check all files in the directory
		for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
		{
			std::filesystem::path path = entry.path();

			// if not a file ||
			// already tracked
			// -> skip
			if (std::filesystem::is_directory(path) ||
				Addons.find(path) != Addons.end())
			{
				continue;
			}

			if (path.extension() == dll)
			{
				//QueueAddon(ELoaderAction::Load, path);
				LoadAddon(path);
			}
			else if (path.extension() == dllUpdate)
			{
				path = path.replace_extension("");
				//QueueAddon(ELoaderAction::Reload, path);
				UnloadAddon(path);
				LoadAddon(path);
			}
		}
	}
	
	void UpdateAll()
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (auto& [path, addon] : Addons)
			{
				if (!addon->Definitions) { continue; }

				if (UpdateAddon(path))
				{
					bool wasLoaded = false;
					if (addon->State == EAddonState::Loaded)
					{
						wasLoaded = true;
						UnloadAddon(path);
					}

					UpdateSwapAddon(path);

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

			if (addon->State == EAddonState::Loaded)
			{
				LogWarning(CH_LOADER, "Cancelled loading \"%s\". Already loaded.", strPath);
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
		addon->MD5 = MD5FromFile(aPath);
		addon->Module = LoadLibraryA(strPath);

		/* load lib failed */
		if (!addon->Module)
		{
			LogWarning(CH_LOADER, "Failed LoadLibrary on \"%s\". Incompatible. Last Error: %u", strPath, GetLastError());
			addon->State = EAddonState::Incompatible;
			return;
		}

		/* doesn't have GetAddonDef */
		if (FindFunction(addon->Module, &getAddonDef, "GetAddonDef") == false)
		{
			LogWarning(CH_LOADER, "\"%s\" is not a Nexus-compatible library. Incompatible.", strPath);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			return;
		}
		
		AddonDefinition* tmpDefs = getAddonDef();

		if (tmpDefs == nullptr)
		{
			LogWarning(CH_LOADER, "\"%s\" is Nexus-compatible but returned a nullptr. Incompatible.", strPath);
			addon->State = EAddonState::Incompatible;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			return;
		}

		if (addon->State != EAddonState::Reload && UpdateAddon(aPath))
		{
			addon->State = EAddonState::Reload;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			UpdateSwapAddon(aPath);
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
		
		// Allocate new memory and copy data, copy strings
		addon->Definitions = new AddonDefinition(*tmpDefs);
		addon->Definitions->Name = _strdup(tmpDefs->Name);
		addon->Definitions->Author = _strdup(tmpDefs->Author);
		addon->Definitions->Description = _strdup(tmpDefs->Description);
		addon->Definitions->UpdateLink = tmpDefs->UpdateLink ? _strdup(tmpDefs->UpdateLink) : nullptr;

		// if the api doesn't exist and there was one requested
		if (api == nullptr && addon->Definitions->APIVersion != 0)
		{
			LogWarning(CH_LOADER, "Loading was cancelled because \"%s\" requested an API of version %d and no such version exists. Incompatible.", strPath, addon->Definitions->APIVersion);
			addon->State = EAddonState::IncompatibleAPI;
			FreeLibrary(addon->Module);
			addon->Module = nullptr;
			return;
		}

		MODULEINFO moduleInfo{};
		GetModuleInformation(GetCurrentProcess(), addon->Module, &moduleInfo, sizeof(moduleInfo));

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
	void UpdateSwapAddon(const std::filesystem::path& aPath)
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
			LogDebug(CH_LOADER, "%s", fErr.what());
			return;
		}

		LogInfo(CH_LOADER, "Successfully updated %s.", aPath.string().c_str());
	}
	bool UpdateAddon(const std::filesystem::path& aPath)
	{
		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + ".old";
		std::filesystem::path pathUpdate = aPath.string() + ".update";

		if (Addons.find(aPath) == Addons.end())
		{
			return false;
		}

		Addon* addon = Addons[aPath];

		/* cleanup old files */
		if (std::filesystem::exists(pathOld)) { std::filesystem::remove(pathOld); }
		if (std::filesystem::exists(pathUpdate))
		{
			if (addon->MD5 != MD5FromFile(pathUpdate))
			{
				return true;
			}

			std::filesystem::remove(pathUpdate);
		}

		if (!addon->Definitions)
		{
			return false;
		}

		bool wasUpdated = false;

		std::string baseUrl;
		std::string endpoint;

		/* setup baseUrl and endpoint */
		switch (addon->Definitions->Provider)
		{
		case EUpdateProvider::None: return false;

		case EUpdateProvider::Raidcore:
			baseUrl = API_RAIDCORE;
			endpoint = "/addons/" + std::to_string(addon->Definitions->Signature);

			break;

		case EUpdateProvider::GitHub:
			baseUrl = API_GITHUB;
			if (addon->Definitions->UpdateLink == nullptr)
			{
				LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", addon->Definitions->Name);
				return false;
			}

			endpoint = "/repos" + GetEndpoint(addon->Definitions->UpdateLink) + "/releases/latest";

			break;

		case EUpdateProvider::Direct:
			if (addon->Definitions->UpdateLink == nullptr)
			{
				LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", addon->Definitions->Name);
				return false;
			}

			baseUrl = GetBaseURL(addon->Definitions->UpdateLink);
			endpoint = GetEndpoint(addon->Definitions->UpdateLink);

			if (baseUrl.empty() || endpoint.empty())
			{
				return false;
			}

			break;
		}

		/* prepare client request */
		httplib::Client client(baseUrl);

		if (EUpdateProvider::Raidcore == addon->Definitions->Provider)
		{
			auto result = client.Get(endpoint);

			if (!result)
			{
				LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			if (result->status != 200) // not HTTP_OK
			{
				LogWarning(CH_LOADER, "Status %d when fetching %s%s", result->status, baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			json resVersion;
			try
			{
				resVersion = json::parse(result->body);
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_LOADER, "Response from %s%s could not be parsed. Error: %s", baseUrl.c_str(), endpoint.c_str(), ex.what());
				return false;
			}

			if (resVersion.is_null())
			{
				LogWarning(CH_LOADER, "Error parsing API response.");
				return false;
			}

			bool anyNull = false;
			AddonVersion remoteVersion{};
			if (!resVersion["Major"].is_null()) { resVersion["Major"].get_to(remoteVersion.Major); }
			else { anyNull = true; }
			if (!resVersion["Minor"].is_null()) { resVersion["Minor"].get_to(remoteVersion.Minor); }
			else { anyNull = true; }
			if (!resVersion["Build"].is_null()) { resVersion["Build"].get_to(remoteVersion.Build); }
			else { anyNull = true; }
			if (!resVersion["Revision"].is_null()) { resVersion["Revision"].get_to(remoteVersion.Revision); }
			else { anyNull = true; }

			if (anyNull)
			{
				LogWarning(CH_LOADER, "One or more fields in the API response were null.");
				return false;
			}

			if (remoteVersion > addon->Definitions->Version)
			{
				LogInfo(CH_LOADER, "%s is outdated: API replied with Version %s but installed is Version %s", addon->Definitions->Name, remoteVersion.ToString().c_str(), addon->Definitions->Version.ToString().c_str());

				std::string endpointDownload = endpoint + "/download"; // e.g. api.raidcore.gg/addons/17/download

				size_t bytesWritten = 0;
				std::ofstream file(pathUpdate, std::ofstream::binary);
				auto downloadResult = client.Get(endpointDownload, [&](const char* data, size_t data_length) {
					file.write(data, data_length);
					bytesWritten += data_length;
					return true;
					});
				file.close();

				if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
				{
					LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpointDownload.c_str());
					return false;
				}

				LogInfo(CH_LOADER, "Successfully updated %s.", addon->Definitions->Name);
				wasUpdated = true;
			}
		}
		else if (EUpdateProvider::GitHub == addon->Definitions->Provider)
		{
			auto result = client.Get(endpoint);

			if (!result)
			{
				LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			if (result->status != 200) // not HTTP_OK
			{
				LogWarning(CH_LOADER, "Status %d when fetching %s%s", result->status, baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			json response;
			try
			{
				response = json::parse(result->body);
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_LOADER, "Response from %s%s could not be parsed. Error: %s", baseUrl.c_str(), endpoint.c_str(), ex.what());
				return false;
			}

			if (response.is_null())
			{
				LogWarning(CH_LOADER, "Error parsing API response.");
				return false;
			}

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

			if (remoteVersion > addon->Definitions->Version)
			{
				LogInfo(CH_LOADER, "%s is outdated: API replied with Version %s but installed is Version %s", addon->Definitions->Name, remoteVersion.ToString().c_str(), addon->Definitions->Version.ToString().c_str());

				std::string endpointDownload; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll

				if (response["assets"].is_null())
				{
					LogWarning(CH_LOADER, "Release has no assets. Cannot check against version. (%s%s)", baseUrl.c_str(), endpoint.c_str());
					return false;
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

				LogInfo(CH_LOADER, "Successfully updated %s.", addon->Definitions->Name);
				wasUpdated = true;
			}
		}
		else if (EUpdateProvider::Direct == addon->Definitions->Provider)
		{
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

			client.Get(endpointMD5, [&](const char* data, size_t data_length) {
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

			if (md5current == md5remote)
			{
				return false;
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