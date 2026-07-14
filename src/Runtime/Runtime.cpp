///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Runtime.cpp
/// Description  :  Nexus runtime implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Runtime.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <psapi.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <windows.h>

#include "minhook/mh_hook.h"

#include "Engine/Clockwork/Clockwork.h"
namespace Clockwork = Raidcore::Clockwork;

#include "Runtime/Runtime.h"
#include "Core/Hooks/Hooks.h"
#include "Core/Index/Index.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Logging/LogConsole.h"
#include "Engine/Logging/LogWriter.h"
#include "GW2/Multibox/Multibox.h"
#include "res/ResConst.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Resources.h"
#include "Branch.h"
#include "Core/Addons/Addon.h"
#include "Core/Index/Index.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"
#include "Util/Url.h"
#include "Version.h"
#include "Host/Config/CfgManager.h"
#include "Host/Library/LibManager.h"
#include "Core/Index/IdxEnum.h"
#include "Core/Preferences/PrefContext.h"
#include "Network/Updater/Updater.h"
#include "Core/Versioning/Version.h"
#include "Engine/Clockwork/Tasks/CancellationToken.h"
#include "Engine/Clockwork/Tasks/ETaskPriority.h"
#include "Engine/DataLink/DlApi.h"
#include "Host/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Host/Loader/Loader.h"
#include "Engine/Logging/LogEnum.h"
#include "Network/WebRequests/WreClient.h"
#include "Graphics/GrContext.h"
#include "Graphics/Textures/TxLoader.h"
#include "GW2/Gw2Context.h"
#include "Platform/PlContext.h"
#include "Proxy/PxyEnum.h"

namespace Raidcore::Nexus
{
	Runtime& Runtime::Get()
	{
		static Runtime s_Context;
		return s_Context;
	}

	void Runtime::Initialize(EProxyFunction aEntryFunction)
	{
		static EProxyFunction s_EntryFunction = EProxyFunction::NONE;

		/* If an entry function is set, we already initalized. */
		if (s_EntryFunction != EProxyFunction::NONE)
		{
			return;
		}

		s_EntryFunction = aEntryFunction;

		MH_Initialize();
		Hooks::HookIDXGISwapChain();

		Runtime& ctx = Runtime::Get();
		CLogApi* logger = ctx.GetLogger();

		/* Environment info. */
		logger->Info(
			CH_CORE,
			"Game: %s\nModule: %s\nNexus %s %s\nEntry method: %d",
			GetCommandLineA(),
			Index(EPath::NexusDLL).string().c_str(),
			ctx.GetVersion().string().c_str(),
			ctx.GetBuild(),
			aEntryFunction
		);

		/* Initialize self updater here so it can lock this instance and update. */
		ctx.GetSelfUpdater();

		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
		{
			Runtime& ctx = Runtime::Get();
			Resources::Unpack(ctx.Platform().Module(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");
		});

		/* Allocate console logger, if requested. */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			static CConsoleLogger console = CConsoleLogger(ELogLevel::ALL);
			logger->Register(&console);
		}

		/* Multibox-friendly log file. */
		std::filesystem::path logpath;
		if (CmdLine::HasArgument("-mumble"))
		{
			std::string filename = "Nexus_";
			filename.append(CmdLine::GetArgumentValue("-mumble"));
			filename.append(".log");
			logpath = Index(EPath::DIR_NEXUS) / filename;
		}
		else
		{
			logpath = Index(EPath::Log);
		}

		/* Allocate log writer. */
		static CFileLogger writer = CFileLogger(ELogLevel::ALL, logpath);
		logger->Register(&writer);

		/* If running vanilla, do not initialize the hooks and leave the mutex unmodified. */
		if (CmdLine::HasArgument("-ggvanilla"))
		{
			return;
		}

		this->Game().Initialize();
	}

	void Runtime::Shutdown(unsigned int aReason)
	{
		static unsigned int s_ShutdownReason = 0;

		/* If a shutdown reason is set, we already shut down. */
		if (s_ShutdownReason != 0)
		{
			return;
		}

		s_ShutdownReason = aReason;

		std::string reasonStr;
		switch (aReason)
		{
			case 1:          { reasonStr = "Reason: DLL_PROCESS_DETACH"; break; }
			case WM_DESTROY: { reasonStr = "Reason: WM_DESTROY";         break; }
			case WM_CLOSE:   { reasonStr = "Reason: WM_CLOSE";           break; }
			case WM_QUIT:    { reasonStr = "Reason: WM_QUIT";            break; }
			default:
			{
				reasonStr = std::format("Reason: Unknown ({})", aReason);
				break;
			}
		}

		Runtime& ctx = Runtime::Get();
		CLogApi* logger = ctx.GetLogger();
		CUiContext* uictx = ctx.GetUIContext();
		Graphics::TextureLoader& texapi = ctx.Graphics().Textures();

		logger->Critical(CH_CORE, "SHUTDOWN BEGIN | %s", reasonStr.c_str());
		MH_Uninitialize();
		uictx->Shutdown();
		texapi.Shutdown();
		logger->Info(CH_CORE, "SHUTDOWN END");

		/* If we have the window handle and we have an original (target) wndproc. */
		if (ctx.Platform().Window() && Hooks::Target::WndProc)
		{
			/* Reset wndproc back to the original target. */
			SetWindowLongPtr(ctx.Platform().Window(), GWLP_WNDPROC, (LONG_PTR)Hooks::Target::WndProc);
		}

		/* Let the OS take care of freeing the handles. Ugly, but otherwise crashes due to the addon clownfiesta in GW2. */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}

	Version_t const& Runtime::GetVersion()
	{
		static Version_t version =
		{
			V_MAJOR,
			V_MINOR,
			V_BUILD,
			V_REVISION
		};
		return version;
	}

	const char* Runtime::GetBuild()
	{
#ifdef _DEBUG
		return "debug/" BRANCH_NAME;
#else
		return "release/" BRANCH_NAME;
#endif
	}

	Platform::Context& Runtime::Platform()
	{
		return *this->_PlatformContext;
	}

	Host::Context& Runtime::Host()
	{
		return *this->_HostContext;
	}

	Graphics::Context& Runtime::Graphics()
	{
		return *this->_GraphicsContext;
	}

	GW2::Context& Runtime::Game()
	{
		return *this->_GameContext;
	}

	CLogApi* Runtime::GetLogger()
	{
		static CLogApi s_Logger = CLogApi();
		return &s_Logger;
	}

	CDataLinkApi* Runtime::GetDataLink()
	{
		static CDataLinkApi s_DataLinkApi = CDataLinkApi(
			this->GetLogger()
		);
		return &s_DataLinkApi;
	}

	CInputBindApi* Runtime::GetInputBindApi()
	{
		static CInputBindApi s_InputBindApi = CInputBindApi(
			&this->Host().Events(),
			this->GetLogger(),
			Index(EPath::InputBinds)
		);
		return &s_InputBindApi;
	}

	CUiContext* Runtime::GetUIContext()
	{
		static CUiContext s_UiContext = CUiContext(
			this->Graphics().Window(),
			this->GetLogger(),
			this->Graphics().Textures(),
			this->GetDataLink(),
			this->GetInputBindApi(),
			this->Host().Events(),
			this->Game().Mumble()
		);
		return &s_UiContext;
	}

	CSettings* Runtime::GetSettingsCtx()
	{
		static CSettings s_SettingsApi = CSettings(
			Index(EPath::Settings),
			this->GetLogger()
		);
		return &s_SettingsApi;
	}

	Network::CHttpClient* Runtime::GetHttpClient(std::string aURL, bool aDisableCache)
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

		Network::CHttpClient* client = nullptr;

		if (aDisableCache)
		{
			client = new Network::CHttpClient(this->GetLogger(), baseurl);
		}
		else
		{
			std::filesystem::path cachedir = Index(EPath::DIR_COMMON) / baseurl_noprotocol;
			uint32_t cacheLifetime = 30 * 60; // 30 minutes

			if (baseurl == "https://api.raidcore.gg")
			{
				cacheLifetime = 5 * 60; // 5 minutes
			}
			else if (baseurl == "https://api.github.com")
			{
				cacheLifetime = 60 * 60; // 60 minutes
			}

			client = new Network::CHttpClient(this->GetLogger(), baseurl, cachedir, cacheLifetime);
		}

		this->HttpClients.emplace(baseurl_noprotocol, client);

		return client;
	}

	Network::Updater* Runtime::GetSelfUpdater()
	{
		static Network::Updater s_SelfUpdater = Network::Updater(
			this->GetLogger()
		);
		return &s_SelfUpdater;
	}

	Runtime::Runtime()
	{
		Clockwork::Context::Create();

		CreateIndex(GetModuleHandle(NULL));

		this->_PlatformContext = std::make_unique<Platform::Context>(
			Index(EPath::CrashLog),
			Index(EPath::CrashStack)
		);

		this->_HostContext = std::make_unique<Host::Context>(
			*this->GetLogger(),
			Index(EPath::DIR_ADDONS),
			Index(EPath::AddonConfigDefault)
		);

		this->_GraphicsContext = std::make_unique<Graphics::Context>(
			*this->GetLogger(),
			Index(EPath::DIR_TEXTURES)
		);

		this->_GameContext = std::make_unique<GW2::Context>(
			*this->GetDataLink(),
			this->Host().Events(),
			*this->GetLogger(),
			this->Platform().RawInput(),
			this->Platform().Window(),
			*this->GetHttpClient("http://assetcdn.101.arenanetworks.com", /*disablecache=*/ true),
			Index(EPath::GameBinds)
		);
	}

	Runtime::~Runtime()
	{
		if (this->_GameContext)
		{
			this->_GameContext->Shutdown();
			this->_GameContext.reset();
		}

		if (this->_HostContext)
		{
			this->_HostContext->Shutdown();
			this->_HostContext.reset();
		}

		if (this->_PlatformContext)
		{
			this->_PlatformContext->Shutdown();
			this->_PlatformContext.reset();
		}

		const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

		for (auto it = this->HttpClients.begin(); it != this->HttpClients.end();)
		{
			/* Deallocate client. */
			delete it->second;

			/* Erase entry. */
			it = this->HttpClients.erase(it);
		}

		Clockwork::Context::Destroy();
	}
}
