#include "Settings.h"

#include <filesystem>
#include <fstream>

#include "Paths.h"
#include "Consts.h"
#include "Shared.h"

const char* OPT_ACCEPTEULA	= "AcceptEULA";
const char* OPT_DEVMODE		= "DevMode";
const char* OPT_CLOSEMENU	= "CloseMenuAfterSelecting";
const char* OPT_CLOSEESCAPE	= "CloseOnEscape";
const char* OPT_LASTUISCALE	= "LastUIScale";
const char* OPT_FONTSIZE	= "FontSize";
const char* OPT_QAVERTICAL	= "QAVertical";
const char* OPT_QALOCATION	= "QALocation";
const char* OPT_QAOFFSETX	= "QAOffsetX";
const char* OPT_QAOFFSETY	= "QAOffsetY";

namespace Settings
{
	std::mutex	Mutex;
	json		Settings = json::object();

	void Load()
	{
		if (!std::filesystem::exists(Path::F_SETTINGS)) { return; }

		Settings::Mutex.lock();
		{
			try
			{
				std::ifstream file(Path::F_SETTINGS);
				Settings = json::parse(file);
				file.close();
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_CORE, "Settings.json could not be parsed. Error: %s", ex.what());
			}
		}
		Settings::Mutex.unlock();
	}
	void Save()
	{
		Settings::Mutex.lock();
		{
			std::ofstream file(Path::F_SETTINGS);
			file << Settings.dump(1, '\t') << std::endl;
			file.close();
		}
		Settings::Mutex.unlock();
	}
}