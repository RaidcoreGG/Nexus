#include "Main.h"

#include <filesystem>
#include <Psapi.h>
#include <string>

#include "Consts.h"
#include "Hooks.h"
#include "Index.h"
#include "Renderer.h"
#include "resource.h"
#include "Shared.h"
#include "State.h"

#include "Context.h"
#include "Inputs/GameBinds/GameBindsHandler.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"
#include "Services/API/ApiClient.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Logging/CConsoleLogger.h"
#include "Services/Logging/CFileLogger.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Multibox/Multibox.h"
#include "Services/Mumble/Reader.h"
#include "Services/Settings/Settings.h"
#include "Services/Updater/Updater.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

/* entry */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			CContext* ctx = CContext::GetContext();
			ctx->SetModule(hModule);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			/* cleanly remove wndproc hook */
			if (Renderer::WindowHandle && Hooks::GW2::WndProc)
			{
				SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)Hooks::GW2::WndProc);
			}
			break;
		}
	}
	return true;
}

namespace Main
{
	void Initialize(EEntryMethod aEntryMethod)
	{
		if (State::Nexus >= ENexusState::LOAD) { return; }

		State::Nexus = ENexusState::LOAD;

		CContext* ctx = CContext::GetContext();

		//SetUnhandledExceptionFilter(UnhandledExcHandler);
		
		Index::BuildIndex(ctx->GetModule());
		
		CLogHandler* logger = ctx->GetLogger();
		CUpdater* updater = ctx->GetUpdater();

		/* setup default loggers */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			logger->RegisterLogger(new CConsoleLogger(ELogLevel::ALL));
		}
		logger->RegisterLogger(new CFileLogger(ELogLevel::ALL, Index::F_LOG));

		logger->Info(CH_CORE, GetCommandLineA());
		logger->Info(CH_CORE, "%s: %s", Index::F_HOST_DLL != Index::F_CHAINLOAD_DLL ? "Proxy" : "Chainload", Index::F_HOST_DLL.string().c_str());
		logger->Info(CH_CORE, "Nexus: %s %s", ctx->GetVersion().string().c_str(), ctx->GetBuild());
		logger->Info(CH_CORE, "Entry method: %d", aEntryMethod);

		RaidcoreAPI = new CApiClient("https://api.raidcore.gg", true, Index::D_GW2_ADDONS_COMMON_API_RAIDCORE, 30 * 60, 300, 5, 1);//, Certs::Raidcore);
		GitHubAPI = new CApiClient("https://api.github.com", true, Index::D_GW2_ADDONS_COMMON_API_GITHUB, 30 * 60, 60, 60, 60 * 60);

		std::thread([logger, updater]()
		{
			HANDLE hMutex = CreateMutexA(0, true, "RCGG-Mutex-Patch-Nexus");

			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				logger->Info(CH_CORE, "Cannot patch Nexus, mutex locked.");

				/* sanity check to make the compiler happy */
				if (hMutex)
				{
					CloseHandle(hMutex);
				}

				return;
			}

			/* perform/check for update */
			updater->UpdateNexus();

			/* sanity check to make the compiler happy */
			if (hMutex)
			{
				ReleaseMutex(hMutex);
				CloseHandle(hMutex);
			}
		}).detach();

		/* Only initalise if not vanilla */
		if (!CmdLine::HasArgument("-ggvanilla"))
		{
			Multibox::ShareArchive();
			Multibox::ShareLocal();
			Multibox::KillMutex();
			logger->Info(CH_CORE, "Multibox State: %d", Multibox::GetState());

			MH_Initialize();

			ctx->GetMumbleReader();

			Hooks::NexusLink = (NexusLinkData*)ctx->GetDataLink()->GetResource(DL_NEXUS_LINK);
			Hooks::RawInputApi = ctx->GetRawInputApi();
			Hooks::InputBindApi = ctx->GetInputBindApi();
			Hooks::UIContext = ctx->GetUIContext();
			Hooks::TextureService = ctx->GetTextureService();
			Hooks::EventApi = ctx->GetEventApi();

			State::Nexus = ENexusState::LOADED;
		}
		else
		{
			State::Nexus = ENexusState::SHUTDOWN;
		}
	}

	void Shutdown(unsigned int aReason)
	{
		const char* reasonStr;
		switch (aReason)
		{
			case WM_DESTROY: reasonStr = "WM_DESTROY"; break;
			case WM_CLOSE:   reasonStr = "WM_CLOSE"; break;
			case WM_QUIT:    reasonStr = "WM_QUIT"; break;
			default:         reasonStr = NULLSTR; break;
		}

		CContext* ctx = CContext::GetContext();
		CLogHandler* logger = ctx->GetLogger();

		logger->Critical(CH_CORE, String::Format("Main::Shutdown() | Reason: %s", reasonStr).c_str());

		if (State::Nexus < ENexusState::SHUTTING_DOWN)
		{
			State::Nexus = ENexusState::SHUTTING_DOWN;

			// free addons
			Loader::Shutdown();

			ctx->GetUIContext()->Shutdown();
			//delete UIContext;
			//delete MumbleReader;

			// shared mem
			//delete DataLinkService;

			MH_Uninitialize();

			logger->Info(CH_CORE, "Shutdown performed.");

			State::Nexus = ENexusState::SHUTDOWN;
		}

		/* do not free lib, let the OS handle it. else gw2 crashes on shutdown */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}
}
