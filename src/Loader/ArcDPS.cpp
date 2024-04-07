#include "ArcDPS.h"

#include <filesystem>

#include "Consts.h"
#include "core.h"
#include "Paths.h"
#include "Shared.h"
#include "State.h"

#include "resource.h"

#include "Loader.h"

namespace ArcDPS
{
	std::mutex					Mutex;
	HMODULE						ModuleHandle		= nullptr;
	bool						IsLoaded			= false;
	bool						IsBridgeDeployed	= false;
	bool						IsPluginAtlasBuilt	= false;
	std::vector<LibraryAddon*>	PluginLibrary;
	std::vector<int>			Plugins;

	addextension2				exp_addextension2	= nullptr;
	listextension				exp_listextension	= nullptr;

	void GetPluginLibrary()
	{
		json response = RaidcoreAPI->Get("/arcdpslibrary");

		if (!response.is_null())
		{
			const std::lock_guard<std::mutex> lock(Mutex);
			PluginLibrary.clear();

			for (const auto& addon : response)
			{
				LibraryAddon* newAddon = new LibraryAddon{};
				newAddon->Signature = addon["id"];
				newAddon->Name = addon["name"];
				newAddon->Description = addon["description"];
				newAddon->Provider = GetProvider(addon["download"]);
				newAddon->DownloadURL = addon["download"];
				if (addon.contains("tos_compliance") && !addon["tos_compliance"].is_null())
				{
					newAddon->ToSComplianceNotice = addon["tos_compliance"];
				}

				PluginLibrary.push_back(newAddon);
			}

			std::sort(PluginLibrary.begin(), PluginLibrary.end(), [](LibraryAddon* a, LibraryAddon* b) {
				return a->Name < b->Name;
				});
		}
		else
		{
			LogWarning(CH_CORE, "Error parsing API response for /arcdpslibrary.");
		}
	}

	void Detect()
	{
		if (IsLoaded)
		{
			return;
		}

		// The following code is a bit ugly and repetitive, but this is because for each of these dlls you need to check whether it is arcdps

		if (State::IsChainloading && std::filesystem::exists(Path::F_CHAINLOAD_DLL))
		{
			HMODULE hModule = GetModuleHandle(Path::F_CHAINLOAD_DLL.string().c_str());

			void* func = nullptr;

			if (hModule)
			{
				if (true == FindFunction(hModule, &func, "addextension2"))
				{
					ModuleHandle = hModule;
					IsLoaded = true;

					LogInfo("ArcDPS", "ArcDPS is not loaded as Nexus addon but was detected as Chainload.");

					DeployBridge();
					return;
				}
			}
		}

		std::filesystem::path alArc = Path::D_GW2_ADDONS / "arcdps" / "gw2addon_arcdps.dll";
		if (std::filesystem::exists(alArc))
		{
			HMODULE hModule = GetModuleHandle(alArc.string().c_str());

			void* func = nullptr;

			if (hModule)
			{
				if (true == FindFunction(hModule, &func, "addextension2"))
				{
					ModuleHandle = hModule;
					IsLoaded = true;

					LogInfo("ArcDPS", "ArcDPS is not loaded as Nexus addon but was detected as Addon-Loader addon.");

					DeployBridge();
					return;
				}
			}
		}

		std::filesystem::path proxyArc = Path::D_GW2 / "d3d11.dll";
		if (std::filesystem::exists(proxyArc))
		{
			HMODULE hModule = GetModuleHandle(proxyArc.string().c_str());

			void* func = nullptr;

			if (hModule)
			{
				if (true == FindFunction(hModule, &func, "addextension2"))
				{
					ModuleHandle = hModule;
					IsLoaded = true;

					LogInfo("ArcDPS", "ArcDPS is not loaded as Nexus addon but was detected as Proxy.");

					DeployBridge();
					return;
				}
			}
		}
	}
	void DeployBridge()
	{
		/* write bridge to disk and queue load */
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
			Loader::QueueAddon(ELoaderAction::Reload, Path::F_ARCDPSINTEGRATION);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			LogDebug("ArcDPS", "%s", fErr.what());
			return;
		}
	}
	void InitializeBridge(HMODULE aBridgeModule)
	{
		if (IsBridgeDeployed || !aBridgeModule || !IsLoaded)
		{
			return;
		}

		/* arcdps integration detection & deploy */
		if (true == FindFunction(ModuleHandle, &exp_addextension2, "addextension2"))
		{
			int result = exp_addextension2(aBridgeModule);
			LogInfo("ArcDPS", "Deployed ArcDPS Integration. Result: %d", result);
		}
		else
		{
			LogWarning("ArcDPS", "Addon with signature \"0xFFF694D1\" found but \"addextension2\" is not exported. ArcDPS combat events won't be relayed.");
		}

		if (true == FindFunction(ModuleHandle, &exp_listextension, "listextension"))
		{
			GetPlugins();
			LogInfo("ArcDPS", "Received ArcDPS plugins.");
		}
		else
		{
			LogWarning("ArcDPS", "Addon with signature \"0xFFF694D1\" found but \"listextension\" is not exported.");
		}

		IsBridgeDeployed = true;
	}

	void GetPlugins()
	{
		if (exp_listextension)
		{
			exp_listextension(AddToAtlas);
		}
	}
	void AddToAtlas(arcdps_exports* aArcdpsExports)
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		if (std::find(Plugins.begin(), Plugins.end(), aArcdpsExports->sig) == Plugins.end())
		{
			Plugins.push_back(aArcdpsExports->sig);
		}

		IsPluginAtlasBuilt = true;
	}
	void Add(HMODULE aModule)
	{
		if (exp_addextension2)
		{
			int result = exp_addextension2(aModule);
			if (result != 0)
			{
				LogDebug("ArcDPS", "Could not add extension. Error %d", result);
			}
		}
	}
}
