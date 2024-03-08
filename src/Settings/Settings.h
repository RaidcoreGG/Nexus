#ifndef SETTINGS_H
#define SETTINGS_H

#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

extern const char* OPT_LASTGAMEBUILD;
extern const char* OPT_LASTCHECKEDGAMEBUILD;
extern const char* OPT_ACCEPTEULA;
extern const char* OPT_NOTIFYCHANGELOG;
extern const char* OPT_DEVMODE;
extern const char* OPT_CLOSEMENU;
extern const char* OPT_CLOSEESCAPE;
extern const char* OPT_LASTUISCALE;
extern const char* OPT_FONTSIZE;
extern const char* OPT_QAVERTICAL;
extern const char* OPT_QALOCATION;
extern const char* OPT_QAOFFSETX;
extern const char* OPT_QAOFFSETY;
extern const char* OPT_LINKARCSTYLE;
extern const char* OPT_IMGUISTYLE;
extern const char* OPT_IMGUICOLORS;
extern const char* OPT_LANGUAGE;

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