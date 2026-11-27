///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.cpp
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Context.h"

#include <psapi.h>

#include "Branch.h"
#include "Host/GW2/Addons/Addon.h"
#include "Host/GW2/Index/Index.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"
#include "Util/Url.h"
#include "Version.h"

namespace Raidcore::GW2
{
	CContext* CContext::GetContext()
	{
		static CContext s_Context;
		return &s_Context;
	}

	Core::Version_t const& CContext::GetVersion()
	{
		static Core::Version_t version =
		{
			V_MAJOR,
			V_MINOR,
			V_BUILD,
			V_REVISION
		};
		return version;
	}

	const char* CContext::GetBuild()
	{
#ifdef _DEBUG
		return "debug/" BRANCH_NAME;
#else
		return "release/" BRANCH_NAME;
#endif
	}

	void CContext::SetModule(HMODULE aModule)
	{
		assert(!this->Module);
		this->Module = aModule;

		MODULEINFO moduleInfo{};
		GetModuleInformation(GetCurrentProcess(), this->Module, &moduleInfo, sizeof(moduleInfo));
		this->ModuleSize = moduleInfo.SizeOfImage;
	}

	HMODULE CContext::GetModule()
	{
		return this->Module;
	}

	DWORD CContext::GetModuleSize()
	{
		return this->ModuleSize;
	}

	Core::Windows::CCrashHandler* CContext::GetCrashHandler()
	{
		static Core::Windows::CCrashHandler s_CrashHandler = Core::Windows::CCrashHandler(
			GW2::Index(GW2::EPath::CrashLog),
			GW2::Index(GW2::EPath::CrashStack)
		);
		return &s_CrashHandler;
	}

	UI::RenderContext_t* CContext::GetRendererCtx()
	{
		static UI::RenderContext_t s_RendererCtx = UI::RenderContext_t();
		return &s_RendererCtx;
	}

	UI::CTextureLoader* CContext::GetTextureService()
	{
		static UI::CTextureLoader s_TextureApi = UI::CTextureLoader(
			this->GetRendererCtx(),
			GW2::Index(GW2::EPath::DIR_TEXTURES)
		);
		return &s_TextureApi;
	}

	Engine::CDataLinkApi* CContext::GetDataLink()
	{
		static Engine::CDataLinkApi s_DataLinkApi = Engine::CDataLinkApi();
		return &s_DataLinkApi;
	}

	Engine::CEventApi* CContext::GetEventApi()
	{
		static Engine::CEventApi s_EventApi = Engine::CEventApi(
			this->GetLoader()
		);
		return &s_EventApi;
	}

	GW2::CEventRouter* CContext::GetEventRouter()
	{
		static GW2::CEventRouter s_EventRouter = GW2::CEventRouter(
			this->GetEventApi()
		);
		return &s_EventRouter;
	}

	Engine::CLoader* CContext::GetLoader()
	{
		static Engine::CLoader s_Loader = Engine::CLoader(
			GW2::IAddonFactory,
			GW2::Index(GW2::EPath::DIR_ADDONS)
		);
		return &s_Loader;
	}

	GW2::CLibraryMgr* CContext::GetAddonLibrary()
	{
		static GW2::CLibraryMgr s_LibraryMgr = GW2::CLibraryMgr(
			this->GetLoader()
		);
		return &s_LibraryMgr;
	}

	Engine::Input::CRawInputApi* CContext::GetRawInputApi()
	{
		static Engine::Input::CRawInputApi s_RawInputApi = Engine::Input::CRawInputApi();
		return &s_RawInputApi;
	}

	Engine::Input::CInputBindApi* CContext::GetInputBindApi()
	{
		static Engine::Input::CInputBindApi s_InputBindApi = Engine::Input::CInputBindApi(
			this->GetEventApi(),
			GW2::Index(GW2::EPath::InputBinds)
		);
		return &s_InputBindApi;
	}

	GW2::CGameBindsApi* CContext::GetGameBindsApi()
	{
		static GW2::CGameBindsApi s_GameBindsApi = GW2::CGameBindsApi(
			this->GetRawInputApi(),
			this->GetEventApi(),
			this->GetRendererCtx(),
			GW2::Index(GW2::EPath::GameBinds)
		);
		return &s_GameBindsApi;
	}

	GW2::CMouseResetFix* CContext::GetMouseResetFix()
	{
		static GW2::CMouseResetFix s_MouseResetFix = GW2::CMouseResetFix();
		return &s_MouseResetFix;
	}

	GW2::Hooks::CGameWindow* CContext::GetGameWindow()
	{
		static GW2::Hooks::CGameWindow s_GameWindow = GW2::Hooks::CGameWindow(
			this->GetRendererCtx(),
			this->GetLoader(),
			this->GetDataLink(),
			this->GetEventApi()
		);
		return &s_GameWindow;
	}

	UI::CUiContext* CContext::GetUIContext()
	{
		static UI::CUiContext s_UiContext = UI::CUiContext(
			this->GetRendererCtx(),
			this->GetTextureService(),
			this->GetDataLink(),
			this->GetInputBindApi(),
			this->GetMumbleReader()
		);
		return &s_UiContext;
	}

	GW2::CMumbleReader* CContext::GetMumbleReader()
	{
		static GW2::CMumbleReader s_MumbleReader = GW2::CMumbleReader(
			this->GetDataLink(),
			this->GetEventApi()
		);
		return &s_MumbleReader;
	}

	Engine::Networking::CHttpClient* CContext::GetHttpClient(std::string aURL, bool aDisableCache)
	{
		const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

		std::string baseurl = URL::GetBase(aURL);

		std::string baseurl_noprotocol = baseurl;
		baseurl_noprotocol = String::Replace(baseurl_noprotocol, "http://", "");
		baseurl_noprotocol = String::Replace(baseurl_noprotocol, "https://", "");

		auto it = this->HttpClients.find(baseurl_noprotocol);

		if (it != this->HttpClients.end())
		{
			return it->second;
		}

		std::filesystem::path cachedir = GW2::Index(GW2::EPath::DIR_COMMON) / baseurl_noprotocol;
		uint32_t cacheLifetime = 30 * 60; // 30 minutes

		if (baseurl == "https://api.raidcore.gg")
		{
			cacheLifetime = 5 * 60; // 5 minutes
		}
		else if (baseurl == "https://api.github.com")
		{
			cacheLifetime = 60 * 60; // 60 minutes
		}

		Engine::Networking::CHttpClient* client = aDisableCache
			? new Engine::Networking::CHttpClient(baseurl)
			: new Engine::Networking::CHttpClient(baseurl, cachedir, cacheLifetime);

		if (this->IsHttpStopped)
		{
			client->Stop();
		}

		this->HttpClients.emplace(baseurl_noprotocol, client);

		return client;
	}

	GW2::CSelfUpdater* CContext::GetSelfUpdater()
	{
		static GW2::CSelfUpdater s_SelfUpdater = GW2::CSelfUpdater();
		return &s_SelfUpdater;
	}

	GW2::ArcDPS::CArcApi* CContext::GetArcApi()
	{
		static GW2::ArcDPS::CArcApi s_ArcApi = GW2::ArcDPS::CArcApi();
		return &s_ArcApi;
	}

	GW2::CConfigMgr* CContext::GetCfgMgr()
	{
		static std::filesystem::path cfgpath      = GW2::Index(GW2::EPath::AddonConfigDefault);
		static std::vector<uint32_t> cfgwhitelist = {};

		static bool s_CmdLineParsed = [this] {
			if (CmdLine::HasArgument("-ggaddons"))
			{
				std::vector<std::string> idList = String::Split(CmdLine::GetArgumentValue("-ggaddons"), ",");

				/* If only one entry and it contains ".json" it's a custom config. */
				if (idList.size() == 1 && String::Contains(idList[0], ".json"))
				{
					cfgpath = idList[0];
				}
				else
				{
					for (std::string addonsig : idList)
					{
						try
						{
							uint32_t sig = std::stoi(addonsig, nullptr, 0);
							cfgwhitelist.push_back(sig);
						}
						catch (const std::invalid_argument& e)
						{
							Core::LogTrace("Loader", "Invalid argument(-ggaddons) : %s (exc: %s)", addonsig.c_str(), e.what());
						}
						catch (const std::out_of_range& e)
						{
							Core::LogTrace("Loader", "Out of range (-ggaddons): %s (exc: %s)", addonsig.c_str(), e.what());
						}
					}
				}
			}
			return true;
		}();

		static GW2::CConfigMgr s_CfgMgr = GW2::CConfigMgr(
			cfgpath,
			cfgwhitelist
		);
		return &s_CfgMgr;
	}

	void CContext::StopHttpClients()
	{
		const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

		this->IsHttpStopped = true;

		for (auto& [url, client] : this->HttpClients)
		{
			client->Stop();
		}
	}

	CContext::~CContext()
	{
		const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

		for (auto it = this->HttpClients.begin(); it != this->HttpClients.end();)
		{
			/* Deallocate client. */
			delete it->second;

			/* Erase entry. */
			it = this->HttpClients.erase(it);
		}
	}
}
