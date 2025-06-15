///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Updater.h
/// Description  :  Handles Nexus & addon updates, as well as addon installation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef UPDATER_H
#define UPDATER_H

#include <filesystem>
#include <string>
#include <vector>

#include "Core/Addons/Library/LibAddon.h"
#include "Engine/Loader/AddonDefinition.h"
#include "Engine/Logging/LogApi.h"

constexpr const char* CH_UPDATER = "Updater";

///----------------------------------------------------------------------------------------------------
/// AddonInfo_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonInfo_t
{
	signed int                 Signature;
	std::string                Name;
	AddonVersion_t             Version;
	EUpdateProvider            Provider;
	std::string                UpdateLink;
	std::vector<unsigned char> MD5;
	bool                       AllowPrereleases;
};

///----------------------------------------------------------------------------------------------------
/// CUpdater Class
///----------------------------------------------------------------------------------------------------
class CUpdater
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CUpdater(CLogApi* aLogger);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CUpdater() = default;

	///----------------------------------------------------------------------------------------------------
	/// UpdateNexus:
	/// 	Checks for an update for Nexus.
	///----------------------------------------------------------------------------------------------------
	void UpdateNexus();

	///----------------------------------------------------------------------------------------------------
	/// UpdateAddon:
	/// 	Checks for an update for an addon.
	///----------------------------------------------------------------------------------------------------
	bool UpdateAddon(const std::filesystem::path& aPath, AddonInfo_t aAddonInfo, bool aIgnoreTagFormat = false, int aCacheLifetimeOverride = -1);

	///----------------------------------------------------------------------------------------------------
	/// InstallAddon:
	/// 	Installs an addon and notifies the loader.
	///----------------------------------------------------------------------------------------------------
	bool InstallAddon(LibraryAddon_t aAddon, bool aIsArcPlugin = false);

	///----------------------------------------------------------------------------------------------------
	/// IsUpdateAvailable:
	/// 	Returns whether an update is available.
	///----------------------------------------------------------------------------------------------------
	bool IsUpdateAvailable();

	///----------------------------------------------------------------------------------------------------
	/// GetChangelog:
	/// 	Returns the changelog.
	///----------------------------------------------------------------------------------------------------
	const std::string& GetChangelog();

	private:
	CLogApi* Logger = nullptr;

	bool UpdateAvailable = false;
	std::string Changelog;

	///----------------------------------------------------------------------------------------------------
	/// UpdateRaidcore:
	/// 	Downloads the latest addon available at the remote, if it's newer than current.
	///----------------------------------------------------------------------------------------------------
	bool UpdateRaidcore(int aCacheLifetimeOverride = -1);

	///----------------------------------------------------------------------------------------------------
	/// UpdateGitHub:
	/// 	Downloads the latest addon available at the remote, if it's newer than current.
	///----------------------------------------------------------------------------------------------------
	bool UpdateGitHub(std::filesystem::path& aDownloadPath, std::string& aEndpoint, AddonVersion_t aCurrentVersion, bool aAllowPrereleases, bool aIgnoreTagFormat = false, int aCacheLifetimeOverride = -1);

	///----------------------------------------------------------------------------------------------------
	/// UpdateDirect:
	/// 	Downloads the latest addon available at the remote, if it's newer than current.
	///----------------------------------------------------------------------------------------------------
	bool UpdateDirect(std::filesystem::path& aDownloadPath, std::string& aBaseURL, std::string& aEndpointDownload, std::vector<unsigned char> aCurrentMD5);

	///----------------------------------------------------------------------------------------------------
	/// UpdateSelf:
	/// 	Downloads the addon available at remote without checking its version.
	/// 	The addon already did that.
	/// 	I hope.
	///----------------------------------------------------------------------------------------------------
	bool UpdateSelf(std::filesystem::path& aDownloadPath, std::string& aBaseURL, std::string& aEndpointDownload);
};

#endif
