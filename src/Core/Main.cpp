///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Main.cpp
/// Description  :  Primary Nexus entry point.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Main.h"

#include <string>
#include <thread>

#include "minhook/mh_hook.h"

#include "Engine/Clockwork/Clockwork.h"
namespace Clockwork = Raidcore::Clockwork;

#include "Core/Context.h"
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
#include "Util/Strings.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			CContext* ctx = CContext::GetContext();
			ctx->SetModule(hModule);
			CreateIndex(hModule);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			CContext* ctx = CContext::GetContext();

			Main::Shutdown(1);

			/* If we have the window handle and we have an original (target) wndproc. */
			if (ctx->GetRendererCtx()->Window.Handle && Hooks::Target::WndProc)
			{
				/* Reset wndproc back to the original target. */
				SetWindowLongPtr(ctx->GetRendererCtx()->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Hooks::Target::WndProc);
			}
			break;
		}
	}
	return true;
}

namespace Main
{
	void Initialize(EProxyFunction aEntryFunction)
	{
		static EProxyFunction s_EntryFunction = EProxyFunction::NONE;

		/* If an entry function is set, we already initalized. */
		if (s_EntryFunction != EProxyFunction::NONE)
		{
			return;
		}

		s_EntryFunction = aEntryFunction;

		Clockwork::Context::Create();

		CContext* ctx    = CContext::GetContext();
		CLogApi*  logger = ctx->GetLogger();

		/* Environment info. */
		logger->Info(
			CH_CORE,
			"Game: %s\nModule: %s\nNexus %s %s\nEntry method: %d",
			GetCommandLineA(),
			Index(EPath::NexusDLL).string().c_str(),
			ctx->GetVersion().string().c_str(),
			ctx->GetBuild(),
			aEntryFunction
		);

		/* Initialize crash handler. */
		CCrashHandler* crashhandler = ctx->GetCrashHandler();

		/* Initialize self updater here so it can lock this instance and update. */
		CSelfUpdater* selfupdater = ctx->GetSelfUpdater();

		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
		{
			CContext* ctx = CContext::GetContext();
			Resources::Unpack(ctx->GetModule(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");
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
			CContext* ctx = CContext::GetContext();
			CLibraryMgr* libmgr = ctx->GetAddonLibrary();
			libmgr->AddSource("https://api.raidcore.gg/addonlibrary");
			libmgr->AddSource("https://api.raidcore.gg/arcdpslibrary");
			libmgr->Update();
		});

		/* Prefetch game build. */
		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Immediate, [](Clockwork::CancellationToken aToken)
		{
			GW2::GetGameBuild();
		});

		MH_Initialize();
		Hooks::HookIDXGISwapChain();

		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [](Clockwork::CancellationToken aToken)
		{
			CContext* ctx = CContext::GetContext();
			CLogApi* logger = ctx->GetLogger();
			
			Multibox::KillMutex();
			logger->Info(CH_CORE, "Multibox State: %d", Multibox::GetState());
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

		Clockwork::Context::Destroy();

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

		CContext*       ctx    = CContext::GetContext();
		CLogApi*        logger = ctx->GetLogger();
		CUiContext*     uictx  = ctx->GetUIContext();
		CTextureLoader* texapi = ctx->GetTextureService();

		logger->Critical(CH_CORE, "SHUTDOWN BEGIN | %s", reasonStr.c_str());
		MH_Uninitialize();
		uictx->Shutdown();
		texapi->Shutdown();
		logger->Info(CH_CORE, "SHUTDOWN END");

		/* Let the OS take care of freeing the handles. Ugly, but otherwise crashes due to the addon clownfiesta in GW2. */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}
}
