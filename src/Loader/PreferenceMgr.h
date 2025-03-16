///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PreferenceMgr.h
/// Description  :  Loader component for managing addon preferences.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef PREFERENCEMGR_H
#define PREFERENCEMGR_H

#include <mutex>
#include <unordered_map>
#include <filesystem>

#include "AddonPreferences.h"
#include "Services/Logging/LogHandler.h"

///----------------------------------------------------------------------------------------------------
/// CPreferenceMgr Class
///----------------------------------------------------------------------------------------------------
class CPreferenceMgr
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CPreferenceMgr(CLogHandler* aLogger, std::filesystem::path aConfigPath, bool aReadOnly = false);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CPreferenceMgr();

	///----------------------------------------------------------------------------------------------------
	/// LoadPrefsSafe:
	/// 	Loads the addon preferences from disk. Threadsafe.
	///----------------------------------------------------------------------------------------------------
	void LoadPrefsSafe();

	///----------------------------------------------------------------------------------------------------
	/// SavePrefsSafe:
	/// 	Saves the addon preferences to disk. Threadsafe.
	///----------------------------------------------------------------------------------------------------
	void SavePrefsSafe();

	///----------------------------------------------------------------------------------------------------
	/// LoadPrefs:
	/// 	Loads the addon preferences from disk. Not threadsafe.
	///----------------------------------------------------------------------------------------------------
	void LoadPrefs();

	///----------------------------------------------------------------------------------------------------
	/// SavePrefs:
	/// 	Saves the addon preferences to disk. Not threadsafe.
	///----------------------------------------------------------------------------------------------------
	void SavePrefs();

	///----------------------------------------------------------------------------------------------------
	/// RegisterPrefs:
	/// 	Returns the preferences of the given addon signature or registers them if they don't exist.
	///----------------------------------------------------------------------------------------------------
	AddonPrefs* RegisterPrefs(signed int aSignature);

	///----------------------------------------------------------------------------------------------------
	/// DeletePrefs:
	/// 	Deletes the preferences of the addon.
	///----------------------------------------------------------------------------------------------------
	void DeletePrefs(signed int aSignature);

	private:
	CLogHandler*                               Logger;

	bool                                       ReadOnly;
	std::filesystem::path                      Path;

	std::mutex                                 Mutex;
	std::unordered_map<signed int, AddonPrefs> Preferences;
};

#endif
