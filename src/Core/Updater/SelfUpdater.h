///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  SelfUpdater.h
/// Description  :  Implementation of the self updater.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include <thread>
#include <string>
#include <windows.h>

#include "Core/Versioning/VerU64_4XS16.h"
#include "Engine/Logging/LogApi.h"

constexpr const char* CH_SELFUPDATER = "Updater";

///----------------------------------------------------------------------------------------------------
/// CSelfUpdater Class
///----------------------------------------------------------------------------------------------------
class CSelfUpdater
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CSelfUpdater(CLogApi* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CSelfUpdater();

	///----------------------------------------------------------------------------------------------------
	/// GetRemoteVersion:
	/// 	Gets the latest available version.
	///----------------------------------------------------------------------------------------------------
	const MajorMinorBuildRevision_t& GetRemoteVersion();

	///----------------------------------------------------------------------------------------------------
	/// IsUpdateAvailable:
	/// 	Returns true if current version is outdated.
	///----------------------------------------------------------------------------------------------------
	bool IsUpdateAvailable();

	///----------------------------------------------------------------------------------------------------
	/// GetChangelog:
	/// 	Returns the changelog.
	///----------------------------------------------------------------------------------------------------
	const std::string& GetChangelog();

	private:
	CLogApi*                  Logger = nullptr;

	HANDLE                    UpdateMutex = nullptr;
	std::thread               UpdateThread;

	MajorMinorBuildRevision_t RemoteVersion;
	std::string               Changelog;

	///----------------------------------------------------------------------------------------------------
	/// CreatePatchMutex:
	/// 	Creates the system-wide mutex that locks the dll file.
	/// 	Returns true on success, false on failure.
	/// 	If the creation fails, no patch should be performed.
	///----------------------------------------------------------------------------------------------------
	bool CreatePatchMutex();

	///----------------------------------------------------------------------------------------------------
	/// CleanupUpdateFiles:
	/// 	Ensures d3d11.dll.old and d3d11.dll.update are usable.
	///----------------------------------------------------------------------------------------------------
	void CleanupUpdateFiles();

	///----------------------------------------------------------------------------------------------------
	/// DownloadUpdate:
	/// 	Returns true if the update downloaded successfully.
	///----------------------------------------------------------------------------------------------------
	bool DownloadUpdate();

	///----------------------------------------------------------------------------------------------------
	/// Runs:
	/// 	Checks if an update is available and installs it.
	///----------------------------------------------------------------------------------------------------
	void Run();
};

#endif
