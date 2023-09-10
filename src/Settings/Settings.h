#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <mutex>
#include <fstream>

#include "../Paths.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

extern const char* OPT_DEVMODE;
extern const char* OPT_LASTUISCALE;
extern const char* OPT_FONTSIZE;
extern const char* OPT_QAVERTICAL;
extern const char* OPT_QALOCATION;
extern const char* OPT_QAOFFSETX;
extern const char* OPT_QAOFFSETY;

namespace Settings
{
	extern std::mutex	Mutex;
	extern json			Settings;

	/* Loads the settings. */
	void Load();
	/* Saves the settings. */
	void Save();
}

#endif