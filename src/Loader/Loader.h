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
	/// 	Responsible for processing "/addon" directory updates.
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
	/// IsValid:
	/// 	Returns true if the provided path is a valid addon path.
	/// 	Validity criteria: exists, not a directory, filesize > 0, .dll extension.
	/// 	Symlinks are followed.
	///----------------------------------------------------------------------------------------------------
	bool IsValid(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// IsTrackedSafe:
	/// 	Returns true if the provided path is an already tracked addon. Threadsafe.
	///----------------------------------------------------------------------------------------------------
	bool IsTrackedSafe(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// IsTrackedSafe:
	/// 	Returns true if the provided MD5 is an already tracked addon. Threadsafe.
	///----------------------------------------------------------------------------------------------------
	bool IsTrackedSafe(const std::vector<unsigned char>& aMD5);

	///----------------------------------------------------------------------------------------------------
	/// IsTracked:
	/// 	Returns true if the provided path is an already tracked addon. Not threadsafe.
	///----------------------------------------------------------------------------------------------------
	bool IsTracked(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// IsTracked:
	/// 	Returns true if the provided MD5 is an already tracked addon. Not threadsafe.
	///----------------------------------------------------------------------------------------------------
	bool IsTracked(const std::vector<unsigned char>& aMD5);

	///----------------------------------------------------------------------------------------------------
	/// GetAddonInterfaces:
	/// 	Returns the interfaces of the given DLL.
	///----------------------------------------------------------------------------------------------------
	EAddonInterface GetAddonInterfaces(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetGameBuild:
	/// 	Gets the current game build from the API.
	///----------------------------------------------------------------------------------------------------
	void GetGameBuild();

	///----------------------------------------------------------------------------------------------------
	/// GetAddonSafe:
	/// 	Returns the addon with the library def. Threadsafe.
	///----------------------------------------------------------------------------------------------------
	CAddon* GetAddonSafe(LibraryAddon* aLibraryDef);

};

#endif
