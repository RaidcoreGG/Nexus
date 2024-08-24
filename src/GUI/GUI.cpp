#ifdef MEME

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

#include "Branch.h"
#include "Consts.h"
#include "Index.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"
#include "Version.h"

#include "Services/Mumble/Reader.h"

#include "Events/EventHandler.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Settings/Settings.h"
#include "Services/Textures/TextureLoader.h"

#include "imgui/imgui.h"

#include "Widgets/Menu/Menu.h"
#include "Widgets/Menu/MenuItem.h"

#include "resource.h"
#include "Util/Base64.h"

namespace GUI
{
	std::map<EFont, ImFont*>				FontIndex;
	std::string								FontFile;
	float									FontSize					= 16.0f;
	bool									CloseMenuAfterSelecting		= true;
	bool									LinkArcDPSStyle				= true;

	bool									IsSetup						= false;
	float									LastScaling					= 1.0f;

	bool									HasAcceptedEULA				= false;
	bool									NotifyChangelog				= false;

	bool									ShowAddonsWindowAfterDUU	= false;

	void ProcessInputBind(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		if (str == KB_MENU)
		{
			Menu::Visible = !Menu::Visible;
			return;
		}
		else if (str == KB_TOGGLEHIDEUI)
		{
			IsUIVisible = !IsUIVisible;
			return;
		}
		else if (str == KB_ADDONS)
		{
			AddonsWindow->Visible = !AddonsWindow->Visible;
		}
		else if (str == KB_DEBUG)
		{
			DebugWindow->Visible = !DebugWindow->Visible;
		}
		else if (str == KB_LOG)
		{
			LogWindow->Visible = !LogWindow->Visible;
		}
		else if (str == KB_OPTIONS)
		{
			OptionsWindow->Visible = !OptionsWindow->Visible;
		}
		else if (str == KB_MUMBLEOVERLAY)
		{
			DebugWindow->MumbleWindow->Visible = !DebugWindow->MumbleWindow->Visible;
		}
	}

	void Setup()
	{
		Language->Advance(); // advance once to build lang atlas prior to creation of Quick Access

		ImGuiIO& io = ImGui::GetIO();

		

		StyleColorsRaidcoreNexus();
		ImportArcDPSStyle();

		LoadFonts();

		EventApi->Subscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityChanged, true);
		EventApi->Subscribe("EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED", OnLanguageChanged, true);
		EventApi->Subscribe(EV_VOLATILE_ADDON_DISABLED, OnAddonDUU, true);
		OnMumbleIdentityChanged(nullptr);

		AddonsWindow	= new CAddonsWindow("Addons");
		OptionsWindow	= new COptionsWindow("Options");
		LogWindow		= new CLogWindow("Log", ELogLevel::ALL);
		DebugWindow		= new CDebugWindow("Debug");
		AboutWindow		= new CAboutBox("About");

		Logger->RegisterLogger(GUI::LogWindow);

		/* add menu items */
		Menu::AddMenuItem("((000083))",		ICON_RETURN,	RES_ICON_RETURN,	&Menu::Visible);
		Menu::AddMenuItem("((000003))",		ICON_ADDONS,	RES_ICON_ADDONS,	&AddonsWindow->Visible);
		Menu::AddMenuItem("((000004))",		ICON_OPTIONS,	RES_ICON_OPTIONS,	&OptionsWindow->Visible);
		Menu::AddMenuItem("((000006))",		ICON_LOG,		RES_ICON_LOG,		&LogWindow->Visible);
		if (State::IsDeveloperMode)
		{
			Menu::AddMenuItem("((000007))", ICON_DEBUG, RES_ICON_DEBUG, &DebugWindow->Visible);
		}
		Menu::AddMenuItem("((000008))",		ICON_ABOUT,		RES_ICON_ABOUT,		&AboutWindow->Visible);

		/* register close on escape */
		RegisterCloseOnEscape("Menu", &Menu::Visible);
		RegisterCloseOnEscape("Addons", &AddonsWindow->Visible);
		RegisterCloseOnEscape("Options", &OptionsWindow->Visible);
		RegisterCloseOnEscape("Log", &LogWindow->Visible);
		RegisterCloseOnEscape("Debug", &DebugWindow->Visible);
		RegisterCloseOnEscape("Dear ImGui Metrics/Debugger", &DebugWindow->IsMetricsWindowVisible);
		RegisterCloseOnEscape("Memory Viewer", &DebugWindow->MemoryViewer.Open);
		RegisterCloseOnEscape("About", &AboutWindow->Visible);
		
		/* register InputBinds */
		InputBindApi->Register(KB_MENU, EIBHType::DownOnly, ProcessInputBind, "CTRL+O");
		InputBindApi->Register(KB_ADDONS, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_OPTIONS, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_LOG, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_DEBUG, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_MUMBLEOVERLAY, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_TOGGLEHIDEUI, EIBHType::DownOnly, ProcessInputBind, "CTRL+H");

		IsSetup = true;
	}
}

#endif
