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

#include "thirdparty/Clockwork/Clockwork.h"
namespace Clockwork = Raidcore::Clockwork;

#include "Runtime/Runtime.h"
#include "Hooks/Hooks.h"
#include "Core/Index/Index.h"
#include "Core/Logging/LogApi.h"
#include "Core/Logging/LogConsole.h"
#include "Core/Logging/LogWriter.h"
#include "GW2/Multibox/Multibox.h"
#include "res/ResConst.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Resources.h"
#include "Branch.h"
#include "Host/Addons/Addon.h"
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
#include "thirdparty/Clockwork/Tasks/CancellationToken.h"
#include "thirdparty/Clockwork/Tasks/ETaskPriority.h"
#include "Core/DataLink/DlApi.h"
#include "Host/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Host/Loader/Loader.h"
#include "Core/Logging/LogEnum.h"
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
		CLogApi& logger = ctx.Core().Logger();

		/* Environment info. */
		logger.Info(
			CH_CORE,
			"Game: %s\nModule: %s\nNexus %s %s\nEntry method: %d",
			GetCommandLineA(),
			Index(EPath::NexusDLL).string().c_str(),
			ctx.GetVersion().string().c_str(),
			ctx.GetBuild(),
			aEntryFunction
		);

		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
		{
			Runtime& ctx = Runtime::Get();
			Resources::Unpack(ctx.Platform().Module(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");
		});

		/* Allocate console logger, if requested. */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			static CConsoleLogger console = CConsoleLogger(ELogLevel::ALL);
			logger.Register(&console);
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
		logger.Register(&writer);

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
		CLogApi& logger = ctx.Core().Logger();
		GUI::CUiContext* uictx = ctx.GetUIContext();
		Graphics::TextureLoader& texapi = ctx.Graphics().Textures();

		logger.Critical(CH_CORE, "SHUTDOWN BEGIN | %s", reasonStr.c_str());
		MH_Uninitialize();
		uictx->Shutdown();
		texapi.Shutdown();
		logger.Info(CH_CORE, "SHUTDOWN END");

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

	Core::Context& Runtime::Core()
	{
		return *this->_CoreContext;
	}

	Network::Context& Runtime::Network()
	{
		return *this->_NetworkContext;
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

	CInputBindApi* Runtime::GetInputBindApi()
	{
		static CInputBindApi s_InputBindApi = CInputBindApi(
			&this->Host().Events(),
			&this->Core().Logger(),
			Index(EPath::InputBinds)
		);
		return &s_InputBindApi;
	}

	GUI::CUiContext* Runtime::GetUIContext()
	{
		static GUI::CUiContext s_UiContext = GUI::CUiContext(
			this->Graphics().Window(),
			&this->Core().Logger(),
			this->Graphics().Textures(),
			&this->Core().DataLink(),
			this->GetInputBindApi(),
			this->Host().Events(),
			this->Game().Mumble()
		);
		return &s_UiContext;
	}

	Runtime::Runtime()
	{
		Clockwork::Context::Create();

		CreateIndex(GetModuleHandle(NULL));

		this->_CoreContext = std::make_unique<Core::Context>(
			Index(EPath::Settings)
		);

		this->_NetworkContext = std::make_unique<Network::Context>(
			this->Core().Logger(),
			Index(EPath::DIR_COMMON)
		);

		this->_PlatformContext = std::make_unique<Platform::Context>(
			Index(EPath::CrashLog),
			Index(EPath::CrashStack)
		);

		this->_HostContext = std::make_unique<Host::Context>(
			this->Core().Logger(),
			Index(EPath::DIR_ADDONS),
			Index(EPath::AddonConfigDefault)
		);

		this->_GraphicsContext = std::make_unique<Graphics::Context>(
			this->Core().Logger(),
			Index(EPath::DIR_TEXTURES)
		);

		this->_GameContext = std::make_unique<GW2::Context>(
			this->Core().DataLink(),
			this->Host().Events(),
			this->Core().Logger(),
			this->Platform().RawInput(),
			this->Platform().Window(),
			this->Network().GetHttpClient("http://assetcdn.101.arenanetworks.com", /*disablecache=*/ true),
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

		Clockwork::Context::Destroy();
	}
}
