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

#include "Loader/AddonDefinition.h"
#include "Loader/LibraryAddon.h"

constexpr const char* CH_UPDATER = "Updater";

///----------------------------------------------------------------------------------------------------
/// AddonInfo Struct
///----------------------------------------------------------------------------------------------------
struct AddonInfo
{
	signed int					Signature;
	std::string					Name;
	AddonVersion				Version;
	EUpdateProvider				Provider;
	std::string					UpdateLink;
	std::vector<unsigned char>	MD5;
};

///----------------------------------------------------------------------------------------------------
/// Updater Namespace
///----------------------------------------------------------------------------------------------------
namespace Updater
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RequestUpdate:
	/// 	Addon API wrapper function for -
	///----------------------------------------------------------------------------------------------------
	const char* ADDONAPI_RequestUpdate(const char* aUpdateURL);
}

///----------------------------------------------------------------------------------------------------
/// CUpdater Singleton
///----------------------------------------------------------------------------------------------------
class CUpdater
{
public:
	///----------------------------------------------------------------------------------------------------
	/// GetInstance:
	/// 	Returns a reference to an instance of CUpdater.
	///----------------------------------------------------------------------------------------------------
	static CUpdater& GetInstance();

	///----------------------------------------------------------------------------------------------------
	/// UpdateNexus:
	/// 	Checks for an update for Nexus.
	///----------------------------------------------------------------------------------------------------
	void UpdateNexus();

	///----------------------------------------------------------------------------------------------------
	/// UpdateAddon:
	/// 	Checks for an update for an addon.
	///----------------------------------------------------------------------------------------------------
	bool UpdateAddon(const std::filesystem::path& aPath, AddonInfo aAddonInfo, bool aAllowPrereleases = false, bool aIgnoreTagFormat = false);

	///----------------------------------------------------------------------------------------------------
	/// InstallAddon:
	/// 	Installs an addon and notifies the loader.
	///----------------------------------------------------------------------------------------------------
	bool InstallAddon(LibraryAddon* aAddon, bool aIsArcPlugin = false);

	CUpdater(CUpdater const&) = delete;
	void operator=(CUpdater const&) = delete;

private:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CUpdater() = default;
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CUpdater() = default;

	///----------------------------------------------------------------------------------------------------
	/// UpdateRaidcore:
	/// 	Downloads the latest addon available at the remote, if it's newer than current.
	///----------------------------------------------------------------------------------------------------
	bool UpdateRaidcore(void);

	///----------------------------------------------------------------------------------------------------
	/// UpdateGitHub:
	/// 	Downloads the latest addon available at the remote, if it's newer than current.
	///----------------------------------------------------------------------------------------------------
	bool UpdateGitHub(std::filesystem::path& aDownloadPath, std::string& aEndpoint, AddonVersion aCurrentVersion, bool aAllowPrereleases, bool aIgnoreTagFormat = false);

	///----------------------------------------------------------------------------------------------------
	/// UpdateDirect:
	/// 	Downloads the latest addon available at the remote, if it's newer than current.
	///----------------------------------------------------------------------------------------------------
	bool UpdateDirect(std::filesystem::path& aDownloadPath, std::string& aBaseURL, std::string& aEndpointDownload, std::vector<unsigned char> aCurrentMD5);
};

#endif