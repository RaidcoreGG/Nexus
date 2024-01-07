#include "Main.h"

#include <filesystem>

#include "Consts.h"
#include "Hooks.h"
#include "Paths.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"

#include "DataLink/DataLink.h"
#include "GUI/GUI.h"
#include "Keybinds/KeybindHandler.h"
#include "Loader/Loader.h"
#include "Logging/LogHandler.h"
#include "Mumble/Mumble.h"
#include "Settings/Settings.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "httpslib.h"

/* entry */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		NexusHandle = hModule;
		break;
	case DLL_PROCESS_DETACH: break;
	}
	return true;
}

namespace Main
{
	void Initialize()
	{
		if (State::Nexus >= ENexusState::LOAD) { return; }

		State::Nexus = ENexusState::LOAD;

		//SetUnhandledExceptionFilter(UnhandledExcHandler);
		GameHandle = GetModuleHandle(NULL);

		Path::Initialize(NexusHandle);
		LogHandler::Initialize();

		LogInfo(CH_CORE, GetCommandLineA());
		LogInfo(CH_CORE, "Version: %s", Version.ToString().c_str());
		LogInfo(CH_CORE, "Main::Initialize() called. Entry method: %d", State::EntryMethod);

		State::Initialize();

		//Paradigm::Initialize();
		SelfUpdate();

		/* Don't initialize anything if vanilla */
		if (!State::IsVanilla)
		{
			MH_Initialize();

			Keybinds::Initialize();
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

			Mumble::Initialize();

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
		switch (aReason)
		{
		case WM_DESTROY:
			LogCritical(CH_CORE, "Main::Shutdown() | Reason: WM_DESTROY");
			break;
		case WM_CLOSE:
			LogCritical(CH_CORE, "Main::Shutdown() | Reason: WM_CLOSE");
			break;
		case WM_QUIT:
			LogCritical(CH_CORE, "Main::Shutdown() | Reason: WM_QUIT");
			break;
		}

		if (State::Nexus < ENexusState::SHUTTING_DOWN)
		{
			State::Nexus = ENexusState::SHUTTING_DOWN;

			// free addons
			Loader::Shutdown();

			GUI::Shutdown();
			Mumble::Shutdown();

			// shared mem
			DataLink::Free();

			/* Save keybinds, settings, api keys & api cache */
			Keybinds::Save();
			Settings::Save();
			//API::Save();

			MH_Uninitialize();

			LogInfo(CH_CORE, "Shutdown performed.");

			SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)Hooks::GW2::WndProc);

			State::Nexus = ENexusState::SHUTDOWN;
		}

		// free libs
		// FIXME: make arc not shit itself when the game shuts down, for now let windows handle the rest
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}

	void SelfUpdate()
	{
		if (std::filesystem::exists(Path::F_OLD_DLL))
		{
			std::filesystem::remove(Path::F_OLD_DLL);
		}

		httplib::Client client(API_RAIDCORE);
		auto result = client.Get("/nexusversion.json");

		if (!result)
		{
			LogWarning(CH_CORE, "Error fetching %s%s", API_RAIDCORE, "/nexusversion.json");
			return;
		}

		if (result->status != 200) // not HTTP_OK
		{
			LogWarning(CH_CORE, "Status %d when fetching %s%s", result->status, API_RAIDCORE, "/nexusversion.json");
			return;
		}

		//LogDebug(CH_CORE, "Body: %s", result->body.c_str());
		json resVersion{};

		try
		{
			resVersion = json::parse(result->body);
		}
		catch (json::parse_error& ex)
		{
			LogWarning(CH_LOADER, "Response from %s%s could not be parsed. Error: %s", API_RAIDCORE, "/nexusversion.json", ex.what());
			return;
		}

		if (resVersion.is_null())
		{
			LogWarning(CH_CORE, "Error parsing API response.");
			return;
		}

		bool anyNull = false;
		AddonVersion remoteVersion{};
		if (!resVersion["Major"].is_null()) { resVersion["Major"].get_to(remoteVersion.Major); }
		else { anyNull = true; }
		if (!resVersion["Minor"].is_null()) { resVersion["Minor"].get_to(remoteVersion.Minor); }
		else { anyNull = true; }
		if (!resVersion["Build"].is_null()) { resVersion["Build"].get_to(remoteVersion.Build); }
		else { anyNull = true; }
		if (!resVersion["Revision"].is_null()) { resVersion["Revision"].get_to(remoteVersion.Revision); }
		else { anyNull = true; }

		if (anyNull)
		{
			LogWarning(CH_CORE, "One or more fields in the API response were null.");
			return;
		}

		if (!resVersion["Changelog"].is_null()) { resVersion["Changelog"].get_to(ChangelogText); }

		if (remoteVersion > Version)
		{
			LogInfo(CH_CORE, "Outdated: API replied with Version %s but installed is Version %s", remoteVersion.ToString().c_str(), Version.ToString().c_str());
			IsUpdateAvailable = true;

			size_t bytesWritten = 0;
			std::ofstream file(Path::F_UPDATE_DLL, std::ofstream::binary);
			auto downloadResult = client.Get("/d3d11.dll", [&](const char* data, size_t data_length) {
				file.write(data, data_length);
				bytesWritten += data_length;
				return true;
				});
			file.close();

			if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
			{
				LogWarning(CH_CORE, "Error fetching %s%s", API_RAIDCORE, "/d3d11.dll");
				return;
			}

			std::filesystem::rename(Path::F_HOST_DLL, Path::F_OLD_DLL);
			std::filesystem::rename(Path::F_UPDATE_DLL, Path::F_HOST_DLL);

			LogInfo(CH_CORE, "Successfully updated Nexus. Restart required to take effect.");
		}
		else if (remoteVersion < Version)
		{
			LogInfo(CH_CORE, "Installed Build of Nexus is more up-to-date than remote. (Installed: %s) (Remote: %s)", Version.ToString().c_str(), remoteVersion.ToString().c_str());
		}
		else
		{
			LogInfo(CH_CORE, "Installed Build of Nexus is up-to-date.");
		}

		if (std::filesystem::exists(Path::F_UPDATE_DLL))
		{
			std::filesystem::remove(Path::F_UPDATE_DLL);
		}
	}
}
