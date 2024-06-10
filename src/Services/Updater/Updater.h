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

#include "Loader/Loader.h"
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
	bool						AllowPrereleases;
};

///----------------------------------------------------------------------------------------------------
/// Updater Namespace
///----------------------------------------------------------------------------------------------------
namespace Updater
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RequestUpdate:
	/// 	Addon API wrapper to self update.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RequestUpdate(signed int aSignature, const char* aUpdateURL);
}

///----------------------------------------------------------------------------------------------------
/// CUpdater Class
///----------------------------------------------------------------------------------------------------
class CUpdater
{
public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CUpdater() = default;
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
	bool UpdateAddon(const std::filesystem::path& aPath, AddonInfo aAddonInfo, bool aIgnoreTagFormat = false);

	///----------------------------------------------------------------------------------------------------
	/// InstallAddon:
	/// 	Installs an addon and notifies the loader.
	///----------------------------------------------------------------------------------------------------
	bool InstallAddon(LibraryAddon* aAddon, bool aIsArcPlugin = false);

private:

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

	///----------------------------------------------------------------------------------------------------
	/// UpdateSelf:
	/// 	Downloads the addon available at remote without checking its version.
	/// 	The addon already did that.
	/// 	I hope.
	///----------------------------------------------------------------------------------------------------
	bool UpdateSelf(std::filesystem::path& aDownloadPath, std::string& aBaseURL, std::string& aEndpointDownload);
};

#endif
