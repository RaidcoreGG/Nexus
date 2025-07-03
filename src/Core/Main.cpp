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

#include "Core/Context.h"
#include "Core/Hooks/Hooks.h"
#include "Core/Index/Index.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Logging/LogConsole.h"
#include "Engine/Logging/LogWriter.h"
#include "GW2/Build/BuildInfo.h"
#include "GW2/Multibox/Multibox.h"
#include "Resources/ResConst.h"
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
	static std::thread s_LibraryThread;
	static std::thread s_BuildThread;
	static std::thread s_LicenseThread;
	static std::thread s_KillMutexThread;

	void Initialize(EProxyFunction aEntryFunction)
	{
		static EProxyFunction s_EntryFunction = EProxyFunction::NONE;

		/* If an entry function is set, we already initalized. */
		if (s_EntryFunction != EProxyFunction::NONE)
		{
			return;
		}

		s_EntryFunction = aEntryFunction;

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

		/* Initialize self updater here so it can lock this instance and update. */
		CSelfUpdater* selfupdater = ctx->GetSelfUpdater();

		s_LicenseThread = std::thread([]() {
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

		s_LibraryThread = std::thread([]() {
			CContext* ctx = CContext::GetContext();
			CLibraryMgr* libmgr = ctx->GetAddonLibrary();
			libmgr->AddSource("https://api.raidcore.gg/addonlibrary");
			libmgr->AddSource("https://api.raidcore.gg/arcdpslibrary");
			libmgr->Update();
		});

		/* Prefetch game build. */
		s_BuildThread = std::thread([]() {
			GW2::GetGameBuild();
		});

		MH_Initialize();

		s_KillMutexThread = std::thread([logger]() {
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

		/* Join threads. */
		if (s_KillMutexThread.joinable()) { s_KillMutexThread.join(); }
		if (s_LicenseThread.joinable()) { s_LicenseThread.join(); }
		if (s_BuildThread.joinable()) { s_BuildThread.join(); }
		if (s_LibraryThread.joinable()) { s_LibraryThread.join(); }

		std::string reasonStr;
		switch (aReason)
		{
			case WM_DESTROY: { reasonStr = "Reason: WM_DESTROY"; break; }
			case WM_CLOSE:   { reasonStr = "Reason: WM_CLOSE";   break; }
			case WM_QUIT:    { reasonStr = "Reason: WM_QUIT";    break; }
			default:
			{
				reasonStr = String::Format("Reason: Unknown (%d)", aReason);
				break;
			}
		}

		CContext*   ctx    = CContext::GetContext();
		CLogApi*    logger = ctx->GetLogger();
		CUiContext* uictx  = ctx->GetUIContext();

		logger->Critical(CH_CORE, "SHUTDOWN BEGIN | %s", reasonStr.c_str());
		MH_Uninitialize();
		logger->Debug(CH_CORE, "SHUTDOWN MID");
		uictx->Shutdown();
		logger->Info(CH_CORE, "SHUTDOWN END");

		/* Let the OS take care of freeing the handles. Ugly, but otherwise crashes due to the addon clownfiesta in GW2. */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}
}
