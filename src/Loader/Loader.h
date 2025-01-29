///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.h
/// Description  :  Addon loader component for managing addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADER_H
#define LOADER_H

#include <filesystem>
#include <mutex>
#include <Shlobj.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Addon.h"
#include "PreferenceMgr.h"
#include "LibraryMgr.h"
#include "LoaderConst.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Settings/Settings.h"

///----------------------------------------------------------------------------------------------------
/// CLoader Class
///----------------------------------------------------------------------------------------------------
class CLoader
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLoader(CLogHandler* aLogger, CSettings* aSettings, std::vector<signed int> aWhitelist = {});

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
	/// GetOwner:
	/// 	Returns the name of the addon owning the address.
	///----------------------------------------------------------------------------------------------------
	std::string GetOwner(void* aAddress);

	private:
	CLogHandler*            Logger;
	CSettings*              Settings;

	int                     GameBuild;

	PIDLIST_ABSOLUTE        FSItemList;
	ULONG                   FSNotifierID;

	std::condition_variable ConVar;
	bool                    IsCanceled;
	std::thread             ProcessorThread;
	int                     ProcessorCountdown;

	std::mutex              AddonsMutex;
	std::vector<CAddon>     Addons;

	CPreferenceMgr*         PreferenceMgr;
	CLibraryMgr*            LibraryMgr;

	///----------------------------------------------------------------------------------------------------
	/// ProcessChanges:
	/// 	Infinite loop that checks for changes to addons.
	///----------------------------------------------------------------------------------------------------
	void ProcessChanges();

	///----------------------------------------------------------------------------------------------------
	/// DetectChanges:
	/// 	Checks for changes to already tracked addons.
	///----------------------------------------------------------------------------------------------------
	void DetectChanges();

	///----------------------------------------------------------------------------------------------------
	/// Discover:
	/// 	Discovers the addons on disk.
	///----------------------------------------------------------------------------------------------------
	void Discover();

	///----------------------------------------------------------------------------------------------------
	/// PathIsValid:
	/// 	Returns true if the provided path is a valid addon path.
	///----------------------------------------------------------------------------------------------------
	bool PathIsValid(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// IsTracked:
	/// 	Returns true if the provided path is an already tracked addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTracked(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetAddonType:
	/// 	Returns the addon type of the given DLL. Caller must ensure pathis a valid addon path.
	///----------------------------------------------------------------------------------------------------
	EAddonType GetAddonType(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetGameBuild:
	/// 	Gets the current game build from the API.
	///----------------------------------------------------------------------------------------------------
	void GetGameBuild();
};

#endif
