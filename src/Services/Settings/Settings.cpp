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

#include "Consts.h"
#include "Context.h"

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
const char* OPT_QAVISIBILITY				= "QAVisibility";
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
const char* OPT_USERFONT					= "UserFont";
const char* OPT_DISABLEFESTIVEFLAIR			= "DisableFestiveFlair";

CSettings::CSettings(std::filesystem::path aPath, CLogHandler* aLogger)
{
	assert(aLogger);

	this->Logger = aLogger;

	this->Path = aPath;
	this->Load();
}

void CSettings::Load()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->LoadInternal();
}

void CSettings::Save()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->SaveInternal();
}

void CSettings::LoadInternal()
{
	if (!std::filesystem::exists(this->Path))
	{
		return;
	}

	try
	{
		std::ifstream file(this->Path);
		this->Store = json::parse(file);
		file.close();
	}
	catch (json::parse_error& ex)
	{
		this->Logger->Warning(CH_SETTINGS, "Settings.json could not be parsed. Error: %s", ex.what());
	}
}

void CSettings::SaveInternal()
{
	try
	{
		std::ofstream file(this->Path);
		file << this->Store.dump(1, '\t') << std::endl;
		file.close();
	}
	catch (...)
	{
		this->Logger->Warning(CH_SETTINGS, "Settings.json could not be saved.");
	}
}
