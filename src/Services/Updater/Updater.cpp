///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Updater.cpp
/// Description  :  Handles Nexus & addon updates, as well as addon installation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Updater.h"

#include <thread>

#include "Index.h"
#include "Shared.h"

#include "Util/MD5.h"
#include "Util/Paths.h"
#include "Util/Strings.h"
#include "Util/URL.h"

namespace Updater
{
	void ADDONAPI_RequestUpdate(signed int aSignature, const char* aUpdateURL)
	{
		if (!aSignature) { return; }
		if (!aUpdateURL) { return; }

		const std::lock_guard<std::mutex> lock(Loader::Mutex);

		Addon* addon = Loader::FindAddonBySig(aSignature);

		if (!addon) { return; }
		if (!addon->Definitions) { return; }
		if (addon->Definitions->Provider != EUpdateProvider::Self)
		{
			Logger->Warning(CH_UPDATER, "Update requested for %s but provider is not EUpdateProvider::Self. Cancelling.", addon->Definitions->Name);
			return;
		}

		/* TODO: Add verification */
		// 1. Addon -> API::RequestUpdate(signature);
		// 2. Nexus -> Events::RaiseSingle(signature, password123);
		// 3. Addon -> API::ConfirmUpdate(password123);

		std::filesystem::path path = addon->Path;

		AddonInfo addonInfo
		{
			addon->Definitions->Signature,
			addon->Definitions->Name,
			addon->Definitions->Version,
			addon->Definitions->Provider,
			aUpdateURL,
			addon->MD5
		};

		std::thread([path, addonInfo]()
			{
				UpdateService->UpdateAddon(path, addonInfo);
			})
			.detach();
	}
}

void CUpdater::UpdateNexus()
{
	// ensure .old path is not claimed
	if (std::filesystem::exists(Index::F_OLD_DLL))
	{
		try
		{
			std::filesystem::remove(Index::F_OLD_DLL);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			std::filesystem::path fallback = Path::GetUnused(Index::F_OLD_DLL);
			std::filesystem::rename(Index::F_OLD_DLL, fallback);

			Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\". Renamed to \"%s\".", Index::F_OLD_DLL.string().c_str(), fallback.string().c_str());
		}
	}

	// ensure .update path is not claimed
	if (std::filesystem::exists(Index::F_UPDATE_DLL))
	{
		try
		{
			std::filesystem::remove(Index::F_UPDATE_DLL);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			std::filesystem::path fallback = Path::GetUnused(Index::F_UPDATE_DLL);
			std::filesystem::rename(Index::F_UPDATE_DLL, fallback);

			Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\". Renamed to \"%s\".", Index::F_UPDATE_DLL.string().c_str(), fallback.string().c_str());
		}
	}

	// check for update and bypass cache
	json resVersion = RaidcoreAPI->Get("/nexusversion", "", 0);

	if (resVersion.is_null())
	{
		Logger->Warning(CH_UPDATER, "Error parsing API response /nexusversion.");
		return;
	}

	// convert to version
	AddonVersion remoteVersion = resVersion;

	// cache changelog for ingame window
	if (!resVersion["Changelog"].is_null())
	{
		resVersion["Changelog"].get_to(ChangelogText);
	}

	if (remoteVersion > Version)
	{
		Logger->Info(CH_UPDATER, "Outdated: API replied with Version %s but installed is Version %s", remoteVersion.string().c_str(), Version.string().c_str());
		IsUpdateAvailable = true;

		// ensure download successful
		if (!RaidcoreAPI->Download(Index::F_UPDATE_DLL, "/d3d11.dll"))
		{
			Logger->Warning(CH_UPDATER, "Nexus Update failed: Download failed.");

			// try cleaning failed download
			if (std::filesystem::exists(Index::F_UPDATE_DLL))
			{
				try
				{
					std::filesystem::remove(Index::F_UPDATE_DLL);
				}
				catch (std::filesystem::filesystem_error fErr)
				{
					Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", Index::F_UPDATE_DLL.string().c_str());
					return;
				}
			}

			return;
		}

		// absolute redundant sanity check
		// above is already ensured that .old is not claimed
		// should it here somehow be claimed anyway, we need to factor that in
		// this ensures that the update can be done
		std::filesystem::path oldPath = Path::GetUnused(Index::F_OLD_DLL);

		// try renaming .dll to .old
		try
		{
			std::filesystem::rename(Index::F_HOST_DLL, oldPath);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Warning(CH_UPDATER, "Nexus update failed: Couldn't move \"%s\" to \"%s\".", Index::F_HOST_DLL.string().c_str(), oldPath.string().c_str());
			return;
		}

		// try renaming .update to .dll
		try
		{
			std::filesystem::rename(Index::F_UPDATE_DLL, Index::F_HOST_DLL);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Warning(CH_UPDATER, "Nexus update failed: Couldn't move \"%s\" to \"%s\".", Index::F_UPDATE_DLL.string().c_str(), Index::F_HOST_DLL.string().c_str());
			return;
		}

		Logger->Info(CH_UPDATER, "Successfully updated Nexus. Restart required to take effect. (Current: %s) (New: %s)", Version.string().c_str(), remoteVersion.string().c_str());
	}
	else if (remoteVersion < Version)
	{
		Logger->Info(CH_UPDATER, "Installed Build of Nexus is more up-to-date than remote. (Installed: %s) (Remote: %s)", Version.string().c_str(), remoteVersion.string().c_str());
	}
	else
	{
		Logger->Info(CH_UPDATER, "Installed Build of Nexus is up-to-date.");
	}
}

bool CUpdater::UpdateAddon(const std::filesystem::path& aPath, AddonInfo aAddonInfo, bool aIgnoreTagFormat, int aCacheLifetimeOverride)
{
	/* setup paths */
	std::filesystem::path pathOld = aPath.string() + ".old";
	std::filesystem::path pathUpdate = aPath.string() + ".update";

	/* cleanup old files */
	try
	{
		if (std::filesystem::exists(pathOld)) { std::filesystem::remove(pathOld); }
	}
	catch (std::filesystem::filesystem_error fErr)
	{
		Logger->Debug(CH_UPDATER, "%s", fErr.what());
		return true; // report as update here, as it was probably moved here during runtime but the dll is locked
	}

	if (std::filesystem::exists(pathUpdate))
	{
		if (aAddonInfo.MD5 != MD5Util::FromFile(pathUpdate))
		{
			return true;
		}

		try
		{
			std::filesystem::remove(pathUpdate);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Debug(CH_UPDATER, "%s", fErr.what());
			return false;
		}
	}

	bool didDownload = false; // didUpdate aka. didDownload

	std::string baseUrl;
	std::string endpoint;

	// override provider if none set, but a Raidcore ID is used
	if (aAddonInfo.Provider == EUpdateProvider::None && aAddonInfo.Signature > 0)
	{
		aAddonInfo.Provider = EUpdateProvider::Raidcore;
	}

	/* setup baseUrl and endpoint */
	switch (aAddonInfo.Provider)
	{
	case EUpdateProvider::None: return false;

	case EUpdateProvider::Raidcore:
		endpoint = "/addons/" + std::to_string(aAddonInfo.Signature);
		break;

	case EUpdateProvider::GitHub:
		if (aAddonInfo.UpdateLink.empty())
		{
			Logger->Warning(CH_UPDATER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", aAddonInfo.Name.c_str());
			return false;
		}

		endpoint = "/repos" + URL::GetEndpoint(aAddonInfo.UpdateLink) + "/releases";
		break;

	case EUpdateProvider::Direct:
		if (aAddonInfo.UpdateLink.empty())
		{
			Logger->Warning(CH_UPDATER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aAddonInfo.Name.c_str());
			return false;
		}

		baseUrl = URL::GetBase(aAddonInfo.UpdateLink);
		endpoint = URL::GetEndpoint(aAddonInfo.UpdateLink);

		if (baseUrl.empty())
		{
			return false;
		}

		break;
	
	case EUpdateProvider::Self:
		if (aAddonInfo.UpdateLink.empty())
		{
			Logger->Warning(CH_UPDATER, "Addon %s declares EUpdateProvider::Self but has no UpdateLink set.", aAddonInfo.Name.c_str());
			return false;
		}

		baseUrl = URL::GetBase(aAddonInfo.UpdateLink);
		endpoint = URL::GetEndpoint(aAddonInfo.UpdateLink);

		if (baseUrl.empty())
		{
			return false;
		}

		break;
	}

	/* this shouldn't happen */
	if (endpoint.empty())
	{
		return false;
	}

	std::filesystem::path tmpPath = aPath.string() + ".update" + ".tmp"; // path that isn't tracked by the Loader like .update is
	tmpPath = Path::GetUnused(tmpPath); // ensure no overlap

	switch (aAddonInfo.Provider)
	{
	case EUpdateProvider::Raidcore:
		if (this->UpdateRaidcore(aCacheLifetimeOverride))
		{
			didDownload = true;
		}
		break;

	case EUpdateProvider::GitHub:
		if (this->UpdateGitHub(tmpPath, endpoint, aAddonInfo.Version, aAddonInfo.AllowPrereleases, aIgnoreTagFormat, aCacheLifetimeOverride))
		{
			didDownload = true;
		}
		break;

	case EUpdateProvider::Direct:
		if (this->UpdateDirect(tmpPath, baseUrl, endpoint, aAddonInfo.MD5))
		{
			didDownload = true;
		}
		break;

	case EUpdateProvider::Self:
		if (this->UpdateSelf(tmpPath, baseUrl, endpoint))
		{
			didDownload = true;
		}
		break;
	}

	if (didDownload)
	{
		// we downloaded something, so we rename from the .tmp or .tmp_# extension back to .update
		try
		{
			std::filesystem::rename(tmpPath, pathUpdate);
			if (aAddonInfo.Version != AddonVersion{}) // if 0.0.0.0 -> install
			{
				Logger->Info(CH_UPDATER, "Successfully updated %s.", aAddonInfo.Name.c_str());
			}
			return true;
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			try
			{
				std::filesystem::remove(tmpPath);
				Logger->Debug(CH_UPDATER, "Error when trying to rename tmp file to update. Deleting. Error: %s", fErr.what());
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				Logger->Debug(CH_UPDATER, "Error when trying to rename tmp file to update. Then another error when deleting. Error: %s", fErr.what());
			}
			return false;
		}
	}

	return false;
}

bool CUpdater::InstallAddon(LibraryAddon* aAddon, bool aIsArcPlugin)
{
	// set state for UI etc
	aAddon->IsInstalling = true;

	AddonInfo addonInfo
	{
		aAddon->Signature,
		aAddon->Name,
		AddonVersion{}, // null version
		aAddon->Provider,
		!aAddon->DownloadURL.empty()
			? aAddon->DownloadURL
			: "",
		{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} // null md5
	};

	std::string filename = String::Normalize(aAddon->Name);
	std::filesystem::path installPath = Index::D_GW2_ADDONS / (filename + ".dll");
	installPath = Path::GetUnused(installPath);

	if (this->UpdateAddon(installPath, addonInfo, aIsArcPlugin))
	{
		try
		{
			std::filesystem::rename(installPath.string() + ".update", installPath);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Debug(CH_UPDATER, "Error when trying to rename update file to dll during installation. Error: %s", fErr.what());
			return false;
		}

		Logger->Info(CH_UPDATER, "Successfully installed %s.", aAddon->Name.c_str());

		return true;
	}

	return false;
}

bool CUpdater::UpdateRaidcore(int aCacheLifetimeOverride)
{
	return false;

	/*json resVersion = RaidcoreAPI->Get(endpoint);

	if (resVersion.is_null())
	{
		Logger->Warning(CH_LOADER, "Error parsing API response.");
		return false;
	}

	AddonVersion remoteVersion = resVersion;

	if (remoteVersion > aVersion)
	{
		Logger->Info(CH_LOADER, "%s is outdated: API replied with Version %s but installed is Version %s", aName.c_str(), remoteVersion.string().c_str(), aVersion.string().c_str());

		RaidcoreAPI->Download(pathUpdate, endpoint + "/download"); // e.g. api.raidcore.gg/addons/17/download

		Logger->Info(CH_LOADER, "Successfully updated %s.", aName.c_str());
		wasUpdated = true;
	}*/
}

bool CUpdater::UpdateGitHub(std::filesystem::path& aDownloadPath, std::string& aEndpoint, AddonVersion aCurrentVersion, bool aAllowPrereleases, bool aIgnoreTagFormat, int aCacheLifetimeOverride)
{
	json response = GitHubAPI->Get(aEndpoint, "", aCacheLifetimeOverride);

	if (response.is_null())
	{
		Logger->Warning(CH_UPDATER, "Error parsing API response.");
		return false;
	}

	std::string targetUrl; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll
	AddonVersion targetVersion{};

	for (json& release : response)
	{
		/* sanity checks */
		if (release["prerelease"].is_null()) { continue; }
		if (!aIgnoreTagFormat && release["tag_name"].is_null()) { continue; }
		if (release["assets"].is_null()) { continue; }

		/* if pre - releases are disabled, but it is one->skip */
		if (!aAllowPrereleases && release["prerelease"].get<bool>()) { continue; }

		AddonVersion version = !aIgnoreTagFormat ? AddonVersion{ release["tag_name"].get<std::string>() } : AddonVersion{ 9999,9999,9999,9999 }; // explicitly get string, otherwise it tries to parse json

		/* skip, if this release we have is the same or older than the one we had found before */
		if (version <= targetVersion) { continue; }

		for (auto& asset : release["assets"])
		{
			/* small sanity check */
			if (asset["name"].is_null()) { continue; }
			if (asset["browser_download_url"].is_null()) { continue; }

			if (String::EndsWith(asset["name"].get<std::string>(), ".dll"))
			{
				targetUrl = asset["browser_download_url"].get<std::string>();
				targetVersion = version;
				break;
			}
		}
	}

	/* if we found no version or URL */
	if (targetVersion == AddonVersion{} ||
		targetUrl.empty())
	{
		return false;
	}

	if (aCurrentVersion >= targetVersion)
	{
		return false;
	}

	std::string downloadBaseUrl = URL::GetBase(targetUrl);
	std::string endpointDownload = URL::GetEndpoint(targetUrl);

	httplib::Client downloadClient(downloadBaseUrl);
	downloadClient.enable_server_certificate_verification(true);
	downloadClient.set_follow_location(true);

	size_t bytesWritten = 0;
	std::ofstream file(aDownloadPath, std::ofstream::binary);
	auto downloadResult = downloadClient.Get(endpointDownload, [&](const char* data, size_t data_length) {
		file.write(data, data_length);
		bytesWritten += data_length;
		return true;
		});
	file.close();

	if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
	{
		Logger->Warning(CH_UPDATER, "Error fetching %s%s", downloadBaseUrl.c_str(), endpointDownload.c_str());

		// try cleaning failed download
		if (std::filesystem::exists(aDownloadPath))
		{
			try
			{
				std::filesystem::remove(aDownloadPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", aDownloadPath.string().c_str());
			}
		}

		return false;
	}

	return true;
}

bool CUpdater::UpdateDirect(std::filesystem::path& aDownloadPath, std::string& aBaseURL, std::string& aEndpointDownload, std::vector<unsigned char> aCurrentMD5)
{
	/* sanity check params */
	if (aDownloadPath.empty() ||
		aBaseURL.empty() ||
		aEndpointDownload.empty() ||
		aCurrentMD5.size() != 16)
	{
		Logger->Warning(CH_UPDATER, "One or more parameters were empty.");
		return false;
	}

	/* prepare client request */
	httplib::Client client(aBaseURL);
	client.enable_server_certificate_verification(true);

	std::string endpointMD5 = aEndpointDownload + ".md5sum";

	std::vector<unsigned char> md5remote = MD5Util::FromRemoteURL(client, endpointMD5);

	// if md5 is incomplete cannot check for update -> false
	// or md5 is same as current -> no update -> false
	if (md5remote.size() != 16 ||
		aCurrentMD5 == md5remote)
	{
		// another inner check if it's a null md5 (install)
		if (aCurrentMD5 != std::vector<unsigned char>{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 })
		{
			return false;
		}
	}

	size_t bytesWritten = 0;
	std::ofstream fileUpdate(aDownloadPath, std::ofstream::binary);
	auto downloadResult = client.Get(aEndpointDownload,
		[&](const char* data, size_t data_length)
		{
			fileUpdate.write(data, data_length);
			bytesWritten += data_length;
			return true;
		});
	fileUpdate.close();

	if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
	{
		Logger->Warning(CH_UPDATER, "Error fetching %s%s", aBaseURL.c_str(), aEndpointDownload.c_str());

		// try cleaning failed download
		if (std::filesystem::exists(aDownloadPath))
		{
			try
			{
				std::filesystem::remove(aDownloadPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", aDownloadPath.string().c_str());
			}
		}

		return false;
	}

	return true;
}

bool CUpdater::UpdateSelf(std::filesystem::path& aDownloadPath, std::string& aBaseURL, std::string& aEndpointDownload)
{
	/* sanity check params */
	if (aDownloadPath.empty() ||
		aBaseURL.empty() ||
		aEndpointDownload.empty())
	{
		Logger->Warning(CH_UPDATER, "One or more parameters were empty.");
		return false;
	}

	/* prepare client request */
	httplib::Client client(aBaseURL);
	client.enable_server_certificate_verification(true);

	size_t bytesWritten = 0;
	std::ofstream fileUpdate(aDownloadPath, std::ofstream::binary);
	auto downloadResult = client.Get(aEndpointDownload,
		[&](const char* data, size_t data_length)
		{
			fileUpdate.write(data, data_length);
			bytesWritten += data_length;
			return true;
		});
	fileUpdate.close();

	if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
	{
		Logger->Warning(CH_UPDATER, "Error fetching %s%s", aBaseURL.c_str(), aEndpointDownload.c_str());

		// try cleaning failed download
		if (std::filesystem::exists(aDownloadPath))
		{
			try
			{
				std::filesystem::remove(aDownloadPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", aDownloadPath.string().c_str());
			}
		}

		return false;
	}

	return true;
}