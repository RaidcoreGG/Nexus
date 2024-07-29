#include "Main.h"

#include <filesystem>
#include <string>
#include <Psapi.h>

#include "resource.h"
#include "Consts.h"
#include "Hooks.h"
#include "Index.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"

#include "Services/DataLink/DataLink.h"
#include "GUI/GUI.h"
#include "Inputs/Keybinds/KeybindHandler.h"
#include "Inputs/GameBinds/GameBindsHandler.h"
#include "Loader/Loader.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Logging/CConsoleLogger.h"
#include "Services/Logging/CFileLogger.h"
#include "Services/Mumble/Reader.h"
#include "Services/Settings/Settings.h"
#include "Services/API/ApiClient.h"
#include "Services/Updater/Updater.h"
#include "Services/Multibox/Multibox.h"

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
			NexusHandle = hModule;
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
	CMumbleReader* MumbleReader;

	void Initialize()
	{
		if (State::Nexus >= ENexusState::LOAD) { return; }

		State::Nexus = ENexusState::LOAD;

		//SetUnhandledExceptionFilter(UnhandledExcHandler);
		GameHandle = GetModuleHandle(NULL);
		MODULEINFO moduleInfo{};
		GetModuleInformation(GetCurrentProcess(), NexusHandle, &moduleInfo, sizeof(moduleInfo));
		NexusModuleSize = moduleInfo.SizeOfImage;

		Index::BuildIndex(NexusHandle);
		std::string mumbleName = State::Initialize();

		/* setup default loggers */
		if (State::IsConsoleEnabled)
		{
			Logger->RegisterLogger(new CConsoleLogger(ELogLevel::ALL));
		}
		Logger->RegisterLogger(new CFileLogger(ELogLevel::ALL, Index::F_LOG));

		Logger->Info(CH_CORE, GetCommandLineA());
		Logger->Info(CH_CORE, "%s: %s", Index::F_HOST_DLL != Index::F_CHAINLOAD_DLL ? "Proxy" : "Chainload", Index::F_HOST_DLL.string().c_str());
		Logger->Info(CH_CORE, "Build: %s", Version.string().c_str());
		Logger->Info(CH_CORE, "Entry method: %d", State::EntryMethod);

		RaidcoreAPI = new CApiClient("https://api.raidcore.gg", true, Index::D_GW2_ADDONS_COMMON_API_RAIDCORE, 30 * 60, 300, 5, 1);//, Certs::Raidcore);
		GitHubAPI = new CApiClient("https://api.github.com", true, Index::D_GW2_ADDONS_COMMON_API_GITHUB, 30 * 60, 60, 60, 60 * 60);

		std::thread([]()
		{
			HANDLE hMutex = CreateMutexA(0, true, "RCGG-Mutex-Patch-Nexus");

			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				Logger->Info(CH_CORE, "Cannot patch Nexus, mutex locked.");

				/* sanity check to make the compiler happy */
				if (hMutex)
				{
					CloseHandle(hMutex);
				}

				return;
			}

			/* perform/check for update */
			UpdateService->UpdateNexus();

			/* sanity check to make the compiler happy */
			if (hMutex)
			{
				ReleaseMutex(hMutex);
				CloseHandle(hMutex);
			}
		})
			.detach();

		//Paradigm::Initialize();

		/* Don't initialize anything if vanilla */
		if (!State::IsVanilla)
		{
			if (Multibox::ShareArchive()) { State::MultiboxState |= EMultiboxState::ARCHIVE_SHARED; }
			if (Multibox::ShareLocal()) { State::MultiboxState |= EMultiboxState::LOCAL_SHARED; }
			if (Multibox::KillMutex()) { State::MultiboxState |= EMultiboxState::MUTEX_CLOSED; }
			Logger->Info(CH_CORE, "Multibox State: %d", State::MultiboxState);

			MH_Initialize();

			KeybindApi = new CKeybindApi();
			GameBindsApi = new CGameBindsApi(RawInputApi, Logger, EventApi);

			Settings::Load();

			// if it's not already been explicitly set via command line, check settings
			if (!State::IsDeveloperMode)
			{
				if (!Settings::Settings[OPT_DEVMODE].is_null())
				{
					Settings::Settings[OPT_DEVMODE].get_to(State::IsDeveloperMode);
				}
				else
				{
					State::IsDeveloperMode = false;
					Settings::Settings[OPT_DEVMODE] = false;
				}
			}

			//API::Initialize();

			MumbleReader = new CMumbleReader(mumbleName);

			// create imgui context
			if (!Renderer::GuiContext) { Renderer::GuiContext = ImGui::CreateContext(); }

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
			case WM_DESTROY:	reasonStr = "WM_DESTROY"; break;
			case WM_CLOSE:		reasonStr = "WM_CLOSE"; break;
			case WM_QUIT:		reasonStr = "WM_QUIT"; break;
			default:			reasonStr = "(null)"; break;
		}
		Logger->Critical(CH_CORE, String::Format("Main::Shutdown() | Reason: %s", reasonStr).c_str());

		if (State::Nexus < ENexusState::SHUTTING_DOWN)
		{
			State::Nexus = ENexusState::SHUTTING_DOWN;

			// free addons
			Loader::Shutdown();

			GUI::Shutdown();
			delete MumbleReader;

			// shared mem
			delete DataLinkService;

			MH_Uninitialize();

			Logger->Info(CH_CORE, "Shutdown performed.");

			State::Nexus = ENexusState::SHUTDOWN;
		}

		/* do not free lib, let the OS handle it. else gw2 crashes on shutdown */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}
}
