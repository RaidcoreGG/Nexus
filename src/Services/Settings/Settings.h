///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Settings.h
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "Services/Logging/LogHandler.h"

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
extern const char* OPT_QAVISIBILITY;
extern const char* OPT_QALOCATION;
extern const char* OPT_QAOFFSETX;
extern const char* OPT_QAOFFSETY;
extern const char* OPT_LINKARCSTYLE;
extern const char* OPT_IMGUISTYLE;
extern const char* OPT_IMGUICOLORS;
extern const char* OPT_LANGUAGE;
extern const char* OPT_ALWAYSSHOWQUICKACCESS;
extern const char* OPT_GLOBALSCALE;
extern const char* OPT_SHOWADDONSWINDOWAFTERDUU;
extern const char* OPT_USERFONT;
extern const char* OPT_DISABLEFESTIVEFLAIR;

constexpr const char* CH_SETTINGS = "Settings";

///----------------------------------------------------------------------------------------------------
/// CSettings Class
///----------------------------------------------------------------------------------------------------
class CSettings
{
	public:
	CSettings(std::filesystem::path aPath, CLogHandler* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the settings.
	///----------------------------------------------------------------------------------------------------
	void Load();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the settings.
	///----------------------------------------------------------------------------------------------------
	void Save();

	template <typename T>
	void Set(std::string aIdentifier, T aValue)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->Store[aIdentifier] = aValue;
		this->SaveInternal();
	}

	template <typename T>
	T Get(const std::string& aIdentifier)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->Store[aIdentifier].is_null())
		{
			return T{};
		}

		return this->Store[aIdentifier].get<T>();
	}

	private:
	CLogHandler*          Logger = nullptr;

	std::filesystem::path Path;

	std::mutex            Mutex;
	json                  Store;

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the settings without locking the mutex.
	///----------------------------------------------------------------------------------------------------
	void LoadInternal();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the settings without locking the mutex.
	///----------------------------------------------------------------------------------------------------
	void SaveInternal();
};

#endif
