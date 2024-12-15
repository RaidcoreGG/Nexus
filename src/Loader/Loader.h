///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.h
/// Description  :  Addon loader component for managing addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <filesystem>
#include <Shlobj.h>

#include "LoaderConst.h"
#include "Addon.h"
#include "AddonPreferences.h"
#include "Services/Logging/LogHandler.h"

///----------------------------------------------------------------------------------------------------
/// CLoader Class
///----------------------------------------------------------------------------------------------------
class CLoader
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLoader(CLogHandler* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLoader();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// NotifyChanges:
	/// 	Notifies the processor of changes to addons.
	///----------------------------------------------------------------------------------------------------
	void NotifyChanges();

	///----------------------------------------------------------------------------------------------------
	/// RegisterPrefs:
	/// 	Returns the preferences of the given addon signature or registered if they don't exist.
	///----------------------------------------------------------------------------------------------------
	AddonPreferences* RegisterPrefs(signed int aSignature);

	///----------------------------------------------------------------------------------------------------
	/// DeregisterPrefs:
	/// 	Deletes the preferences of the addon.
	///----------------------------------------------------------------------------------------------------
	void DeregisterPrefs(signed int aSignature);

	///----------------------------------------------------------------------------------------------------
	/// GetOwner:
	/// 	Returns the name of the addon owning the address.
	///----------------------------------------------------------------------------------------------------
	std::string GetOwner(void* aAddress);

	private:
	CLogHandler*                                     Logger;

	PIDLIST_ABSOLUTE                                 FSItemList;
	ULONG                                            FSNotifierID;

	std::condition_variable                          ConVar;
	bool                                             IsCanceled;
	std::thread                                      ProcessorThread;
	int                                              ProcessorCountdown;

	std::mutex                                       AddonsMutex;
	std::vector<CAddon>                              Addons;

	std::mutex                                       PrefMutex;
	std::unordered_map<signed int, AddonPreferences> Preferences;

	///----------------------------------------------------------------------------------------------------
	/// ProcessChanges:
	/// 	Infinite loop that checks for changes to addons.
	///----------------------------------------------------------------------------------------------------
	void ProcessChanges();

	///----------------------------------------------------------------------------------------------------
	/// Discover:
	/// 	Discovers the addons on disk.
	///----------------------------------------------------------------------------------------------------
	void Discover();

	///----------------------------------------------------------------------------------------------------
	/// IsValid:
	/// 	Returns true if the provided path is a valid addon path.
	///----------------------------------------------------------------------------------------------------
	bool IsValid(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// ProbeAddonType:
	/// 	Returns the addon type of the given DLL. Caller must ensure pathis a valid addon path.
	///----------------------------------------------------------------------------------------------------
	EAddonType ProbeAddonType(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// LoadPrefs:
	/// 	Loads the addon preferences from disk.
	///----------------------------------------------------------------------------------------------------
	void LoadPrefs();

	///----------------------------------------------------------------------------------------------------
	/// GetFlags:
	/// 	Saves the addon preferences to disk.
	///----------------------------------------------------------------------------------------------------
	void SavePrefs();
};

#endif
