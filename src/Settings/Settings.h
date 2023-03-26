#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <mutex>
#include <fstream>

#include "../Paths.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

extern const char* OPT_LASTUISCALE;
extern const char* OPT_FONTSIZE;

namespace Settings
{
	extern std::mutex	Mutex;
	extern json			Settings;

	void Load();
	void Save();
}

#endif