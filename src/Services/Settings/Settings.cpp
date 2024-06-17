///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Settings.cpp
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Settings.h"

#include <filesystem>
#include <fstream>

#include "Index.h"
#include "Consts.h"
#include "Shared.h"

const char* OPT_LASTGAMEBUILD				= "LastGameBuild";
const char* OPT_LASTCHECKEDGAMEBUILD		= "LastCheckedGameBuild";
const char* OPT_ACCEPTEULA					= "AcceptEULA";
const char* OPT_NOTIFYCHANGELOG				= "NotifyChangelog";
const char* OPT_DEVMODE						= "DevMode";
const char* OPT_CLOSEMENU					= "CloseMenuAfterSelecting";
const char* OPT_CLOSEESCAPE					= "CloseOnEscape";
const char* OPT_LASTUISCALE					= "LastUIScale";
const char* OPT_FONTSIZE					= "FontSize";
const char* OPT_QAVERTICAL					= "QAVertical";
const char* OPT_QALOCATION					= "QALocation";
const char* OPT_QAOFFSETX					= "QAOffsetX";
const char* OPT_QAOFFSETY					= "QAOffsetY";
const char* OPT_LINKARCSTYLE				= "LinkWithArcDPSStyle";
const char* OPT_IMGUISTYLE					= "ImGuiStyle";
const char* OPT_IMGUICOLORS					= "ImGuiColors";
const char* OPT_LANGUAGE					= "Language";
const char* OPT_ALWAYSSHOWQUICKACCESS		= "AlwaysShowQuickAccess";
const char* OPT_GLOBALSCALE					= "GlobalScale";
const char* OPT_SHOWADDONSWINDOWAFTERDUU	= "ShowAddonsWindowAfterDisableUntilUpdate";
const char* OPT_NETWORKINGTRIPPLE			= "NetworkingTripple";

namespace Settings
{
	std::mutex	Mutex;
	json		Settings = json::object();

	void Load()
	{
		if (!std::filesystem::exists(Index::F_SETTINGS)) { return; }

		const std::lock_guard<std::mutex> lock(Mutex);
		try
		{
			std::ifstream file(Index::F_SETTINGS);
			Settings = json::parse(file);
			file.close();
		}
		catch (json::parse_error& ex)
		{
			Logger->Warning(CH_CORE, "Settings.json could not be parsed. Error: %s", ex.what());
		}
	}

	void Save()
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		std::ofstream file(Index::F_SETTINGS);
		file << Settings.dump(1, '\t') << std::endl;
		file.close();
	}
}
