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
#include "Engine/CrashHandler/CrashHandler.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Logging/LogConsole.h"
#include "Engine/Logging/LogWriter.h"
#include "GW2/Build/BuildInfo.h"
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
#include "Core/Addons/Config/CfgManager.h"
#include "Core/Addons/Library/LibManager.h"
#include "Core/Index/IdxEnum.h"
#include "Core/Preferences/PrefContext.h"
#include "Core/Updater/SelfUpdater.h"
#include "Core/Versioning/Version.h"
#include "Engine/Clockwork/Tasks/CancellationToken.h"
#include "Engine/Clockwork/Tasks/ETaskPriority.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Logging/LogEnum.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/Renderer/RdrContext.h"
#include "UI/Textures/TxLoader.h"

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

	Clockwork::Context::Create();

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

	/* Initialize crash handler. */
	CCrashHandler* crashhandler = ctx.GetCrashHandler();

	/* Initialize self updater here so it can lock this instance and update. */
	CSelfUpdater* selfupdater = ctx.GetSelfUpdater();

	Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
	{
		Runtime& ctx = Runtime::Get();
		Resources::Unpack(ctx.GetModule(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");
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

	Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Normal, [](Clockwork::CancellationToken aToken)
	{
		Runtime& ctx = Runtime::Get();
		CLibraryMgr* libmgr = ctx.GetAddonLibrary();
		libmgr->AddSource("https://api.raidcore.gg/addonlibrary");
		libmgr->AddSource("https://api.raidcore.gg/arcdpslibrary");
		libmgr->Update();
	});

	/* Prefetch game build. */
	Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Immediate, [](Clockwork::CancellationToken aToken)
	{
		GW2::GetGameBuild();
	});

	Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
	{
		Runtime& ctx = Runtime::Get();
		CLogApi* logger = ctx.GetLogger();

		Multibox::KillMutex();
		logger->Info(CH_CORE, "Multibox State: %d", Multibox::GetState());
	});
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

	Clockwork::Context::Destroy();

	std::string reasonStr;
	switch (aReason)
	{
		case 1: { reasonStr = "Reason: DLL_PROCESS_DETACH"; break; }
		case WM_DESTROY: { reasonStr = "Reason: WM_DESTROY";         break; }
		case WM_CLOSE: { reasonStr = "Reason: WM_CLOSE";           break; }
		case WM_QUIT: { reasonStr = "Reason: WM_QUIT";            break; }
		default:
		{
			reasonStr = std::format("Reason: Unknown ({})", aReason);
			break;
		}
	}

	Runtime& ctx = Runtime::Get();
	CLogApi* logger = ctx.GetLogger();
	CUiContext* uictx = ctx.GetUIContext();
	CTextureLoader* texapi = ctx.GetTextureService();

	logger->Critical(CH_CORE, "SHUTDOWN BEGIN | %s", reasonStr.c_str());
	MH_Uninitialize();
	uictx->Shutdown();
	texapi->Shutdown();
	logger->Info(CH_CORE, "SHUTDOWN END");

	/* If we have the window handle and we have an original (target) wndproc. */
	if (ctx.GetRendererCtx()->Window.Handle && Hooks::Target::WndProc)
	{
		/* Reset wndproc back to the original target. */
		SetWindowLongPtr(ctx.GetRendererCtx()->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Hooks::Target::WndProc);
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

HMODULE Runtime::GetModule()
{
	return this->Module;
}

DWORD Runtime::GetModuleSize()
{
	return this->ModuleSize;
}

GameContext& Runtime::Game()
{
	return *this->_GameContext;
}

CCrashHandler* Runtime::GetCrashHandler()
{
	static CCrashHandler s_CrashHandler = CCrashHandler(
		Index(EPath::CrashLog),
		Index(EPath::CrashStack)
	);
	return &s_CrashHandler;
}

RenderContext_t* Runtime::GetRendererCtx()
{
	static RenderContext_t s_RendererCtx = RenderContext_t();
	return &s_RendererCtx;
}

CLogApi* Runtime::GetLogger()
{
	static CLogApi s_Logger = CLogApi();
	return &s_Logger;
}

CTextureLoader* Runtime::GetTextureService()
{
	static CTextureLoader s_TextureApi = CTextureLoader(
		this->GetLogger(),
		this->GetRendererCtx(),
		Index(EPath::DIR_TEXTURES)
	);
	return &s_TextureApi;
}

CDataLinkApi* Runtime::GetDataLink()
{
	static CDataLinkApi s_DataLinkApi = CDataLinkApi(
		this->GetLogger()
	);
	return &s_DataLinkApi;
}

CEventApi* Runtime::GetEventApi()
{
	static CEventApi s_EventApi = CEventApi(
		this->GetLoader()
	);
	return &s_EventApi;
}

CLoader* Runtime::GetLoader()
{
	static CLoader s_Loader = CLoader(
		this->GetLogger(),
		CAddon::Factory,
		Index(EPath::DIR_ADDONS)
	);
	return &s_Loader;
}

CLibraryMgr* Runtime::GetAddonLibrary()
{
	static CLibraryMgr s_LibraryMgr = CLibraryMgr(
		this->GetLogger(),
		this->GetLoader()
	);
	return &s_LibraryMgr;
}

CRawInputApi* Runtime::GetRawInputApi()
{
	static CRawInputApi s_RawInputApi = CRawInputApi();
	return &s_RawInputApi;
}

CInputBindApi* Runtime::GetInputBindApi()
{
	static CInputBindApi s_InputBindApi = CInputBindApi(
		this->GetEventApi(),
		this->GetLogger(),
		Index(EPath::InputBinds)
	);
	return &s_InputBindApi;
}

CUiContext* Runtime::GetUIContext()
{
	static CUiContext s_UiContext = CUiContext(
		this->GetRendererCtx(),
		this->GetLogger(),
		this->GetTextureService(),
		this->GetDataLink(),
		this->GetInputBindApi(),
		this->GetEventApi(),
		&this->Game().Mumble()
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

CHttpClient* Runtime::GetHttpClient(std::string aURL)
{
	const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

	std::string baseurl = URL::GetBase(aURL);

	std::string baseurl_noprotocol = baseurl;
	baseurl_noprotocol = String::Replace(baseurl_noprotocol, "htts://", "");
	baseurl_noprotocol = String::Replace(baseurl_noprotocol, "https://", "");

	auto it = this->HttpClients.find(baseurl_noprotocol);

	if (it != this->HttpClients.end())
	{
		return it->second;
	}

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

	CHttpClient* client = new CHttpClient(this->GetLogger(), baseurl, cachedir, cacheLifetime);
	this->HttpClients.emplace(baseurl_noprotocol, client);

	return client;
}

CSelfUpdater* Runtime::GetSelfUpdater()
{
	static CSelfUpdater s_SelfUpdater = CSelfUpdater(
		this->GetLogger()
	);
	return &s_SelfUpdater;
}

CConfigMgr* Runtime::GetCfgMgr()
{
	static std::filesystem::path cfgpath      = Index(EPath::AddonConfigDefault);
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
						this->GetLogger()->Trace(CH_LOADER, "Invalid argument(-ggaddons) : %s (exc: %s)", addonsig.c_str(), e.what());
					}
					catch (const std::out_of_range& e)
					{
						this->GetLogger()->Trace(CH_LOADER, "Out of range (-ggaddons): %s (exc: %s)", addonsig.c_str(), e.what());
					}
				}
			}
		}
		return true;
	}();

	static CConfigMgr s_CfgMgr = CConfigMgr(
		this->GetLogger(),
		cfgpath,
		cfgwhitelist
	);
	return &s_CfgMgr;
}

Runtime::Runtime()
{
	HMODULE hmodule = nullptr;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCSTR)&Runtime::Get,
		&hmodule
	);

	MODULEINFO moduleInfo{};
	GetModuleInformation(GetCurrentProcess(), hmodule, &moduleInfo, sizeof(moduleInfo));

	this->Module = hmodule;
	this->ModuleSize = moduleInfo.SizeOfImage;

	CreateIndex(this->Module);

	this->_GameContext = std::make_unique<GameContext>(
		*this->GetDataLink(),
		*this->GetEventApi(),
		*this->GetLogger(),
		*this->GetRawInputApi(),
		*this->GetRendererCtx(),
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

	const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

	for (auto it = this->HttpClients.begin(); it != this->HttpClients.end();)
	{
		/* Deallocate client. */
		delete it->second;

		/* Erase entry. */
		it = this->HttpClients.erase(it);
	}
}
