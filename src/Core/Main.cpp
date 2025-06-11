#include "Main.h"

#include <filesystem>
#include <Psapi.h>
#include <string>
#include <windowsx.h>

#include "Consts.h"
#include "Engine/Index/Index.h"
#include "resource.h"
#include "Shared.h"
#include "State.h"

#include "Core/Context.h"
#include "Core/Hooks/Hooks.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Logging/LogConsole.h"
#include "Engine/Logging/LogWriter.h"
#include "Engine/Logging/LogApi.h"
#include "GW2/Multibox/Multibox.h"
#include "GW2/Mumble/MblReader.h"
#include "GW2/Inputs/MouseResetFix.h"
#include "Engine/Settings/Settings.h"
#include "Engine/Updater/Updater.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"
#include "Util/Resources.h"
#include "Util/Inputs.h"

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
	static NexusLinkData_t*  s_NexusLink      = nullptr;
	static CRawInputApi*     s_RawInputApi    = nullptr;
	static CInputBindApi*    s_InputBindApi   = nullptr;
	static CUiContext*       s_UIContext      = nullptr;
	static CTextureLoader*   s_TextureService = nullptr;
	static CEventApi*        s_EventApi       = nullptr;
	static CSettings*        s_SettingsCtx    = nullptr;
	static RenderContext_t*  s_RendererCtx    = nullptr;

	void Initialize(EProxyFunction aEntryFunction)
	{
		static EProxyFunction s_EntryFunction = EProxyFunction::NONE;

		/* If an entry function is set, we already initalized. */
		if (s_EntryFunction != EProxyFunction::NONE)
		{
			return;
		}

		s_EntryFunction = aEntryFunction;

		if (State::Nexus >= ENexusState::LOAD) { return; }

		State::Nexus = ENexusState::LOAD;

		CContext* ctx = CContext::GetContext();
		s_RendererCtx = ctx->GetRendererCtx();

		Resources::Unpack(ctx->GetModule(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");

		CLogApi* logger = ctx->GetLogger();
		CUpdater* updater = ctx->GetUpdater();

		/* setup default loggers */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			logger->Register(new CConsoleLogger(ELogLevel::ALL));
		}

		std::filesystem::path logPathOverride;

		if (CmdLine::HasArgument("-mumble"))
		{
			logPathOverride = (Index(EPath::DIR_NEXUS) / "Nexus_").string() + CmdLine::GetArgumentValue("-mumble") + ".log";
		}

		logger->Register(new CFileLogger(ELogLevel::ALL, logPathOverride.empty() ? Index(EPath::Log) : logPathOverride));

		logger->Info(CH_CORE, GetCommandLineA());
		logger->Info(CH_CORE, "%s: %s", Index(EPath::NexusDLL) != Index(EPath::D3D11Chainload) ? "Proxy" : "Chainload", Index(EPath::NexusDLL).string().c_str());
		logger->Info(CH_CORE, "Nexus: %s %s", ctx->GetVersion().string().c_str(), ctx->GetBuild());
		logger->Info(CH_CORE, "Entry method: %d", aEntryFunction);

		RaidcoreAPI = new CHttpClient("https://api.raidcore.gg", Index(EPath::DIR_APICACHE_RAIDCORE), 30 * 60, 300, 5, 1);
		GitHubAPI = new CHttpClient("https://api.github.com", Index(EPath::DIR_APICACHE_GITHUB), 30 * 60, 60, 60, 60 * 60);

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
			Multibox::KillMutex();
			logger->Info(CH_CORE, "Multibox State: %d", Multibox::GetState());

			MH_Initialize();

			ctx->GetMumbleReader();

			s_NexusLink = (NexusLinkData_t*)ctx->GetDataLink()->GetResource(DL_NEXUS_LINK);
			s_RawInputApi = ctx->GetRawInputApi();
			s_InputBindApi = ctx->GetInputBindApi();
			s_UIContext = ctx->GetUIContext();
			s_TextureService = ctx->GetTextureService();
			s_EventApi = ctx->GetEventApi();
			s_SettingsCtx = ctx->GetSettingsCtx();

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
		CLogApi* logger = ctx->GetLogger();

		logger->Critical(CH_CORE, String::Format("Main::Shutdown() | Reason: %s", reasonStr).c_str());

		if (State::Nexus < ENexusState::SHUTTING_DOWN)
		{
			State::Nexus = ENexusState::SHUTTING_DOWN;

			// free addons
			Loader::Shutdown();

			ctx->GetUIContext()->Shutdown();

			MH_Uninitialize();

			logger->Info(CH_CORE, "Shutdown performed.");

			State::Nexus = ENexusState::SHUTDOWN;
		}

		/* do not free lib, let the OS handle it. else gw2 crashes on shutdown */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}
}
