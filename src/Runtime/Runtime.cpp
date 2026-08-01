///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Runtime.cpp
/// Description  :  Nexus runtime implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Runtime.h"

#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <windows.h>

#include "minhook/mh_hook.h"

#include "thirdparty/Clockwork/Clockwork.h"
#include "thirdparty/Clockwork/Tasks/CancellationToken.h"
#include "thirdparty/Clockwork/Tasks/ETaskPriority.h"

#include "Branch.h"
#include "Core/DataLink/DlApi.h"
#include "Core/Functions/FnRegistry.h"
#include "Core/Logging/LogApi.h"
#include "Core/Logging/LogConsole.h"
#include "Core/Logging/LogEnum.h"
#include "Core/Logging/LogWriter.h"
#include "Core/Settings/SettingsMgr.h"
#include "Core/Versioning/Version.h"
#include "Graphics/GrMetrics.h"
#include "Graphics/GrWindow.h"
#include "Graphics/Textures/TxLoader.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/BuildInfo/BuildInfoService.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Multibox/Multibox.h"
#include "GW2/Mumble/MblReader.h"
#include "Hooks/Hooks.h"
#include "Host/Addons/Addon.h"
#include "Host/Config/CfgManager.h"
#include "Host/Events/EvtApi.h"
#include "Host/Library/LibManager.h"
#include "Host/Loader/Loader.h"
#include "Index/IdxEnum.h"
#include "Index/Index.h"
#include "Inputs/InputBinds/IbApi.h"
#include "Network/Updater/Updater.h"
#include "Network/WebRequests/WreStorage.h"
#include "Platform/PlContext.h"
#include "Proxy/PxyEnum.h"
#include "res/ResConst.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Dll.h"
#include "Util/Resources.h"
#include "Version.h"

namespace Raidcore::Nexus
{
	constexpr const char* LOG_CHANNEL = "Runtime";

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
		Core::LogApi& logger = ctx.Logger();

		/* Environment info. */
		logger.Info(
			LOG_CHANNEL,
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
			Resources::Unpack(GetCurrentModule(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");
		});

		/* Allocate console logger, if requested. */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			static Core::CConsoleLogger console = Core::CConsoleLogger(Core::ELogLevel::ALL);
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
		static Core::CFileLogger writer = Core::CFileLogger(Core::ELogLevel::ALL, logpath);
		logger.Register(&writer);

		/* If running vanilla, do not initialize the hooks and leave the mutex unmodified. */
		if (CmdLine::HasArgument("-ggvanilla"))
		{
			return;
		}

		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Normal, [this](Clockwork::CancellationToken aToken)
		{
			this->Library().AddSource("https://api.raidcore.gg/addonlibrary");
			this->Library().AddSource("https://api.raidcore.gg/arcdpslibrary");
			this->Library().Update();
		});

		/* Prefetch game build. */
		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Immediate, [this](Clockwork::CancellationToken aToken)
		{
			this->BuildInfo().Build();
		});

		/* Set up multiboxing. */
		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [this](Clockwork::CancellationToken aToken)
		{
			Multibox::KillMutex();
			this->Logger().Info(LOG_CHANNEL, "Multibox State: %d", Multibox::GetState());
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
		Core::LogApi& logger = ctx.Logger();
		GUI::Context& uictx = ctx.UI();
		Graphics::TextureLoader& texapi = ctx.TextureLoader();

		logger.Critical(LOG_CHANNEL, "SHUTDOWN BEGIN | %s", reasonStr.c_str());
		MH_Uninitialize();
		uictx.Shutdown();
		texapi.Shutdown();
		logger.Info(LOG_CHANNEL, "SHUTDOWN END");

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

	Core::LogApi& Runtime::Logger()
	{
		static Core::LogApi s_LogApi{};
		return s_LogApi;
	}

	Core::DataLinkApi& Runtime::DataLink()
	{
		static Core::DataLinkApi s_DataLink{
			this->Logger()
		};
		return s_DataLink;
	}

	Core::FuncRegistry& Runtime::FunctionRegistry()
	{
		static Core::FuncRegistry s_FuncRegistry{
			this->Logger()
		};
		return s_FuncRegistry;
	}

	Core::SettingsMgr& Runtime::Settings()
	{
		static Core::SettingsMgr s_Settings{
			Index(EPath::Settings),
			this->Logger()
		};
		return s_Settings;
	}

	Platform::Context& Runtime::Platform()
	{
		return *this->_PlatformContext;
	}

	Host::ConfigMgr& Runtime::Config()
	{
		static Host::ConfigMgr s_ConfigMgr{
			this->Logger(),
			Index(EPath::AddonConfigDefault)
		};
		return s_ConfigMgr;
	}

	Host::Loader& Runtime::Loader()
	{
		static Host::Loader s_Loader{
			this->Logger(),
			CAddon::Factory, /* FIXME: Register mapping. */
			Index(EPath::DIR_ADDONS)
		};
		return s_Loader;
	}

	Host::LibraryMgr& Runtime::Library()
	{
		static Host::LibraryMgr s_Library{
			this->Logger(),
			this->Loader()
		};
		return s_Library;
	}

	Host::EventApi& Runtime::Events()
	{
		static Host::EventApi s_EventApi{
			this->Loader()
		};
		return s_EventApi;
	}

	Graphics::TextureLoader& Runtime::TextureLoader()
	{
		static Graphics::TextureLoader s_TextureLoader{
			this->Logger(),
			this->GrWindow(),
			Index(EPath::DIR_TEXTURES)
		};
		return s_TextureLoader;
	}

	Graphics::Metrics_t& Runtime::GrMetrics()
	{
		static Graphics::Metrics_t s_Metrics{};
		return s_Metrics;
	}

	Graphics::Window_t& Runtime::GrWindow()
	{
		static Graphics::Window_t s_Window{};
		return s_Window;
	}

	GW2::ArcdpsApi& Runtime::Arcdps()
	{
		static GW2::ArcdpsApi s_ArcdpsApi{};
		return s_ArcdpsApi;
	}

	GW2::BuildInfoService& Runtime::BuildInfo()
	{
		static GW2::BuildInfoService s_BuildInfo{
			this->HttpClientStorage().GetHttpClient("http://assetcdn.101.arenanetworks.com", /*disablecache=*/ true),
			this->Logger()
		};
		return s_BuildInfo;
	}

	GW2::GameBindsApi& Runtime::GameBinds()
	{
		static GW2::GameBindsApi s_GameBinds{
			this->Platform().RawInput(),
			this->Logger(),
			this->Events(),
			this->Platform().Window(),
			Index(EPath::GameBinds)
		};
		return s_GameBinds;
	}

	GW2::MumbleReader& Runtime::Mumble()
	{
		static GW2::MumbleReader s_Mumble{
			this->DataLink(),
			this->Events(),
			this->Logger()
		};
		return s_Mumble;
	}

	Input::CInputBindApi& Runtime::InputBinds()
	{
		static Input::CInputBindApi s_InputBindApi = Input::CInputBindApi(
			&this->Events(),
			&this->Logger(),
			Index(EPath::InputBinds)
		);
		return s_InputBindApi;
	}

	Network::Updater& Runtime::Updater()
	{
		static Network::Updater s_Updater{
			this->Logger()
		};
		return s_Updater;
	}

	Network::ClientStorage& Runtime::HttpClientStorage()
	{
		static Network::ClientStorage s_ClientStorage{
			this->Logger()
		};
		return s_ClientStorage;
	}

	GUI::Context& Runtime::UI()
	{
		static GUI::Context s_UiContext{
			this->Logger(),
			this->DataLink(),
			this->Settings(),
			this->GrWindow(),
			this->TextureLoader(),
			this->InputBinds(),
			this->Events(),
			this->Mumble()
		};

		return s_UiContext;
	}

	Runtime::Runtime()
	{
		Clockwork::Context::Create();

		this->_PlatformContext = std::make_unique<Platform::Context>();
	}

	Runtime::~Runtime()
	{
		if (this->_PlatformContext)
		{
			this->_PlatformContext->Shutdown();
			this->_PlatformContext.reset();
		}

		Clockwork::Context::Destroy();
	}
}
