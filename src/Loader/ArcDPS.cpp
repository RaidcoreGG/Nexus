#include "ArcDPS.h"

#include <filesystem>

#include "Consts.h"
#include "Index.h"
#include "Shared.h"
#include "State.h"

#include "resource.h"

#include "Loader.h"

#include "Util/DLL.h"
#include "Util/Resources.h"

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
				if (addon.contains("addon_policy_tier") && !addon["addon_policy_tier"].is_null())
				{
					newAddon->PolicyTier = addon["addon_policy_tier"];
				}

				PluginLibrary.push_back(newAddon);
			}

			std::sort(PluginLibrary.begin(), PluginLibrary.end(), [](LibraryAddon* a, LibraryAddon* b) {
				return a->Name < b->Name;
				});
		}
		else
		{
			Logger->Warning(CH_CORE, "Error parsing API response for /arcdpslibrary.");
		}
	}

	void Detect()
	{
		if (IsLoaded)
		{
			return;
		}

		// The following code is a bit ugly and repetitive, but this is because for each of these dlls you need to check whether it is arcdps

		if (State::IsChainloading && std::filesystem::exists(Index::F_CHAINLOAD_DLL))
		{
			HMODULE hModule = GetModuleHandleA(Index::F_CHAINLOAD_DLL.string().c_str());

			void* func = nullptr;

			if (hModule)
			{
				if (DLL::FindFunction(hModule, &func, "addextension2"))
				{
					ModuleHandle = hModule;
					IsLoaded = true;

					Logger->Info("ArcDPS", "ArcDPS is not loaded as Nexus addon but was detected as Chainload.");

					DeployBridge();
					return;
				}
			}
		}

		std::filesystem::path alArc = Index::D_GW2_ADDONS / "arcdps" / "gw2addon_arcdps.dll";
		if (std::filesystem::exists(alArc))
		{
			HMODULE hModule = GetModuleHandleA(alArc.string().c_str());

			void* func = nullptr;

			if (hModule)
			{
				if (DLL::FindFunction(hModule, &func, "addextension2"))
				{
					ModuleHandle = hModule;
					IsLoaded = true;

					Logger->Info("ArcDPS", "ArcDPS is not loaded as Nexus addon but was detected as Addon-Loader addon.");

					DeployBridge();
					return;
				}
			}
		}

		std::filesystem::path proxyArc = Index::D_GW2 / "d3d11.dll";
		if (std::filesystem::exists(proxyArc))
		{
			HMODULE hModule = GetModuleHandleA(proxyArc.string().c_str());

			void* func = nullptr;

			if (hModule)
			{
				if (DLL::FindFunction(hModule, &func, "addextension2"))
				{
					ModuleHandle = hModule;
					IsLoaded = true;

					Logger->Info("ArcDPS", "ArcDPS is not loaded as Nexus addon but was detected as Proxy.");

					DeployBridge();
					return;
				}
			}
		}
	}
	void DeployBridge()
	{
		try
		{
			Resources::Unpack(NexusHandle, Index::F_ARCDPSINTEGRATION, RES_ARCDPS_INTEGRATION, "DLL", true);

			// reload is set to true because the dll is always deployed from resource
			// and since it's locked it would not load here directly but instead after checking for updates (which does not apply to it)
			Loader::QueueAddon(ELoaderAction::Reload, Index::F_ARCDPSINTEGRATION);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Debug("ArcDPS", "%s", fErr.what());
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
		if (DLL::FindFunction(ModuleHandle, &exp_addextension2, "addextension2"))
		{
			int result = exp_addextension2(aBridgeModule);
			Logger->Info("ArcDPS", "Deployed ArcDPS Integration. Result: %d", result);
		}
		else
		{
			Logger->Warning("ArcDPS", "Addon with signature \"0xFFF694D1\" found but \"addextension2\" is not exported. ArcDPS combat events won't be relayed.");
		}

		if (DLL::FindFunction(ModuleHandle, &exp_listextension, "listextension"))
		{
			GetPlugins();
			Logger->Info("ArcDPS", "Received ArcDPS plugins.");
		}
		else
		{
			Logger->Warning("ArcDPS", "Addon with signature \"0xFFF694D1\" found but \"listextension\" is not exported.");
		}

		IsBridgeDeployed = true;

		std::thread checkPlugins([]()
			{
				Sleep(5000);
				ArcDPS::GetPlugins();
			});
		checkPlugins.detach();
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
	void AddToAtlasBySig(unsigned int aArcSignature)
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		if (std::find(Plugins.begin(), Plugins.end(), aArcSignature) == Plugins.end())
		{
			Plugins.push_back(aArcSignature);
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
				Logger->Debug("ArcDPS", "Could not add extension. Error %d", result);
			}
		}
	}
}
