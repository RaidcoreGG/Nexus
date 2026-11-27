///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Main.cpp
/// Description  :  Primary Nexus entry point.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Main.h"

#include <string>

#include "minhook/mh_hook.h"

#include "Engine/Clockwork/Clockwork.h"

#include "Host/GW2/Context.h"
#include "Host/GW2/Hooks/Hooks.h"
#include "Host/GW2/Index/Index.h"
#include "Migrations/Migrations.h"
#include "Core/Platform/Windows/Exe/ExeCrashHandler.h"
#include "Core/Logging/LogApi.h"
#include "Core/Logging/Windows/LogConsole.h"
#include "Core/Logging/LogWriter.h"
#include "Core/Preferences/PrefContext.h"
#include "Host/GW2/Build/BuildInfo.h"
#include "Host/GW2/Multibox/Multibox.h"
#include "res/Resources.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			Raidcore::GW2::CContext* ctx = Raidcore::GW2::CContext::GetContext();
			ctx->SetModule(hModule);
			Raidcore::GW2::CreateIndex(hModule);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			Raidcore::GW2::CContext* ctx = Raidcore::GW2::CContext::GetContext();

			Raidcore::GW2::Shutdown(1);

			/* If we have the window handle and we have an original (target) wndproc. */
			if (ctx->GetRendererCtx()->Window.Handle && Raidcore::GW2::Hooks::Target::WndProc)
			{
				/* Reset wndproc back to the original target. */
				SetWindowLongPtr(ctx->GetRendererCtx()->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Raidcore::GW2::Hooks::Target::WndProc);
			}
			break;
		}
	}
	return true;
}

namespace Raidcore::GW2
{
	static bool s_IsInitialized = false;

	void Initialize(Proxy::EProxyFunction aEntryFunction)
	{
		static Proxy::EProxyFunction s_EntryFunction = Proxy::EProxyFunction::NONE;

		/* If an entry function is set, we already initalized. */
		if (s_EntryFunction != Proxy::EProxyFunction::NONE)
		{
			return;
		}

		s_EntryFunction = aEntryFunction;

		if (CmdLine::HasArgument("-ggvanilla"))
		{
			return;
		}

		/* Created first so it is destroyed last: services log from their destructors. */
		Core::CLogApi::Get();

		Migrations::Run();

		/* Loaded before the services that own settings exist; namespace-scope settings take the values here. */
		Core::CPreferences::Get()->Init(GW2::Index(GW2::EPath::Settings));

		MH_Initialize();
		GW2::Hooks::HookIDXGISwapChain();

		Clockwork::Context::Create();

		s_IsInitialized = true;

		GW2::CContext* ctx = GW2::CContext::GetContext();

		/* Environment info. */
		Core::LogInfo(
			GW2::CH_CORE,
			"Game: %s\nModule: %s\nNexus %s %s\nEntry method: %d",
			GetCommandLineA(),
			GW2::Index(GW2::EPath::NexusDLL).string().c_str(),
			ctx->GetVersion().string().c_str(),
			ctx->GetBuild(),
			aEntryFunction
		);

		/* Initialize crash handler. */
		Core::Windows::CCrashHandler* crashhandler = ctx->GetCrashHandler();

		/* Initialize self updater here so it can lock this instance and update. */
		GW2::CSelfUpdater* selfupdater = ctx->GetSelfUpdater();

		Clockwork::Run<void>(Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
		{
			Resources::ThirdPartyNotices.Unpack(GW2::Index(GW2::EPath::ThirdPartySoftwareReadme));
		});

		/* Allocate console logger, if requested. */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			static Core::CConsoleLogger console = Core::CConsoleLogger(Core::ELogLevel::ALL);
			Core::CLogApi::Get()->Register(&console);
		}

		/* Multibox-friendly log file. */
		std::filesystem::path logpath;
		if (CmdLine::HasArgument("-mumble"))
		{
			std::string filename = "Nexus_";
			filename.append(CmdLine::GetArgumentValue("-mumble"));
			filename.append(".log");
			logpath = GW2::Index(GW2::EPath::DIR_NEXUS) / filename;
		}
		else
		{
			logpath = GW2::Index(GW2::EPath::Log);
		}

		/* Allocate log writer. */
#ifdef _DEBUG
		static Core::CFileLogger writer = Core::CFileLogger(Core::ELogLevel::ALL, logpath);
#else
		static Core::CFileLogger writer = Core::CFileLogger(Core::ELogLevel::INFO, logpath);
#endif
		Core::CLogApi::Get()->Register(&writer);

		Clockwork::Run<void>(Clockwork::ETaskPriority::Normal, [](Clockwork::CancellationToken aToken)
		{
			GW2::CContext* ctx = GW2::CContext::GetContext();
			GW2::CLibraryMgr* libmgr = ctx->GetAddonLibrary();
			libmgr->AddSource("https://api.raidcore.gg/addonlibrary");
			libmgr->AddSource("https://api.raidcore.gg/arcdpslibrary");
			libmgr->Update();
		});

		/* Prefetch game build. */
		Clockwork::Run<void>(Clockwork::ETaskPriority::Immediate, [](Clockwork::CancellationToken aToken)
		{
			GW2::GetGameBuild();
		});

		Clockwork::Run<void>(Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
		{
			GW2::Multibox::KillMutex();
			Core::LogInfo(GW2::CH_CORE, "Multibox State: %d", GW2::Multibox::GetState());
		});
	}

	void Shutdown(unsigned int aReason)
	{
		static unsigned int s_ShutdownReason = 0;

		/* If a shutdown reason is set, we already shut down. */
		if (s_ShutdownReason != 0)
		{
			return;
		}

		s_ShutdownReason = aReason;

		if (!s_IsInitialized)
		{
			return;
		}

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

		GW2::CContext*       ctx    = GW2::CContext::GetContext();
		UI::CUiContext*     uictx  = ctx->GetUIContext();
		UI::CTextureLoader* texapi = ctx->GetTextureService();

		Core::LogCritical(GW2::CH_CORE, "SHUTDOWN BEGIN | %s", reasonStr.c_str());

		/* Disabled, not uninitialized: the render thread may still be inside a hook. */
		MH_DisableHook(MH_ALL_HOOKS);

		/* Addons unloading below may still crash or reset the crash filter. */
		ctx->GetCrashHandler()->Reattach();

		/* Requests in flight fail now, so joining the threads below does not wait on the network. */
		ctx->StopHttpClients();

		/* Addons unload while every service they call into still exists. */
		ctx->GetCfgMgr()->SaveConfigs();
		ctx->GetLoader()->Shutdown();

		/* Joins the workers. */
		Clockwork::Context::Destroy();

		uictx->Shutdown();
		uictx->GetFontManager()->InvalidateDeviceObjects();
		texapi->Shutdown();

		Core::LogInfo(GW2::CH_CORE, "SHUTDOWN END");

		/* Let the OS take care of freeing the handles. Ugly, but otherwise crashes due to the addon clownfiesta in GW2. */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}
}
