///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Updater.cpp
/// Description  :  Handles Nexus & addon updates, as well as addon installation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Updater.h"

#include <thread>

#include "Core/Context.h"
#include "Core/Index/Index.h"

#include "Util/MD5.h"
#include "Util/Paths.h"
#include "Util/Strings.h"
#include "Util/URL.h"
#include "Engine/Loader/Loader.h"
#include "Updater.h"
#include "Updater.h"

CUpdater::CUpdater(CLogApi* aLogger)
{
	assert(aLogger);

	this->Logger = aLogger;
}

bool CUpdater::UpdateAddon(const std::filesystem::path& aPath, AddonInfo_t aAddonInfo, bool aIgnoreTagFormat, int aCacheLifetimeOverride)
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
		this->Logger->Debug(CH_UPDATER, "%s", fErr.what());
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
			this->Logger->Debug(CH_UPDATER, "%s", fErr.what());
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
			this->Logger->Warning(CH_UPDATER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", aAddonInfo.Name.c_str());
			return false;
		}

		endpoint = "/repos" + URL::GetEndpoint(aAddonInfo.UpdateLink) + "/releases";
		break;

	case EUpdateProvider::Direct:
		if (aAddonInfo.UpdateLink.empty())
		{
			this->Logger->Warning(CH_UPDATER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aAddonInfo.Name.c_str());
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
			this->Logger->Warning(CH_UPDATER, "Addon %s declares EUpdateProvider::Self but has no UpdateLink set.", aAddonInfo.Name.c_str());
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
			if (aAddonInfo.Version != MajorMinorBuildRevision_t{}) // if 0.0.0.0 -> install
			{
				this->Logger->Info(CH_UPDATER, "Successfully updated %s.", aAddonInfo.Name.c_str());
			}
			return true;
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			try
			{
				std::filesystem::remove(tmpPath);
				this->Logger->Debug(CH_UPDATER, "Error when trying to rename tmp file to update. Deleting. Error: %s", fErr.what());
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Debug(CH_UPDATER, "Error when trying to rename tmp file to update. Then another error when deleting. Error: %s", fErr.what());
			}
			return false;
		}
	}

	return false;
}

bool CUpdater::InstallAddon(LibraryAddon_t aAddon, bool aIsArcPlugin)
{
	// set state for UI etc
	//aAddon->IsInstalling = true;

	AddonInfo_t addonInfo
	{
		aAddon.Signature,
		aAddon.Name,
		MajorMinorBuildRevision_t{}, // null version
		GetProvider(aAddon.DownloadURL),
		!aAddon.DownloadURL.empty()
			? aAddon.DownloadURL
			: "",
		{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} // null md5
	};

	std::string filename = String::Normalize(aAddon.Name);
	std::filesystem::path installPath = Index(EPath::DIR_ADDONS) / (filename + ".dll");
	installPath = Path::GetUnused(installPath);

	if (this->UpdateAddon(installPath, addonInfo, aIsArcPlugin))
	{
		try
		{
			std::filesystem::rename(installPath.string() + ".update", installPath);
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			this->Logger->Debug(CH_UPDATER, "Error when trying to rename update file to dll during installation. Error: %s", fErr.what());
			return false;
		}

		this->Logger->Info(CH_UPDATER, "Successfully installed %s.", aAddon.Name.c_str());

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
		this->Logger->Warning(CH_LOADER, "Error parsing API response.");
		return false;
	}

	MajorMinorBuildRevision_t remoteVersion = resVersion;

	if (remoteVersion > aVersion)
	{
		this->Logger->Info(CH_LOADER, "%s is outdated: API replied with Version %s but installed is Version %s", aName.c_str(), remoteVersion.string().c_str(), aVersion.string().c_str());

		RaidcoreAPI->Download(pathUpdate, endpoint + "/download"); // e.g. api.raidcore.gg/addons/17/download

		this->Logger->Info(CH_LOADER, "Successfully updated %s.", aName.c_str());
		wasUpdated = true;
	}*/
}

bool CUpdater::UpdateGitHub(std::filesystem::path& aDownloadPath, std::string& aEndpoint, MajorMinorBuildRevision_t aCurrentVersion, bool aAllowPrereleases, bool aIgnoreTagFormat, int aCacheLifetimeOverride)
{
	return false;

	HttpResponse_t requestResult = CContext::GetContext()->GetGitHubApi()->Get(aEndpoint, "", aCacheLifetimeOverride);
	json response = requestResult.ContentJSON();

	if (response.is_null() || !requestResult.Success())
	{
		this->Logger->Warning(CH_UPDATER, "Error parsing API response.");
		return false;
	}

	std::string targetUrl; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll
	MajorMinorBuildRevision_t targetVersion{};

	for (json& release : response)
	{
		/* sanity checks */
		if (release["prerelease"].is_null()) { continue; }
		if (!aIgnoreTagFormat && release["tag_name"].is_null()) { continue; }
		if (release["assets"].is_null()) { continue; }

		/* if pre - releases are disabled, but it is one->skip */
		if (!aAllowPrereleases && release["prerelease"].get<bool>()) { continue; }

		MajorMinorBuildRevision_t version = {};//!aIgnoreTagFormat ? MajorMinorBuildRevision_t{ release["tag_name"].get<std::string>() } : MajorMinorBuildRevision_t{ 9999,9999,9999,9999 }; // explicitly get string, otherwise it tries to parse json

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
	if (targetVersion == MajorMinorBuildRevision_t{} ||
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
		this->Logger->Warning(CH_UPDATER, "Error fetching %s%s\nError: %s", downloadBaseUrl.c_str(), endpointDownload.c_str(), httplib::to_string(downloadResult.error()).c_str());
		
		// try cleaning failed download
		if (std::filesystem::exists(aDownloadPath))
		{
			try
			{
				std::filesystem::remove(aDownloadPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", aDownloadPath.string().c_str());
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
		this->Logger->Warning(CH_UPDATER, "One or more parameters were empty.");
		return false;
	}

	/* prepare client request */
	httplib::Client client(aBaseURL);
	client.enable_server_certificate_verification(true);
	client.set_follow_location(true);

	/* null md5 means we're ignoring it (install) */
	if (aCurrentMD5 != std::vector<unsigned char>{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 })
	{
		std::string endpointMD5 = aEndpointDownload + ".md5";
		std::vector<unsigned char> md5remote = MD5Util::FromRemoteURL(client, endpointMD5);

		// if .md5 doesn't exist or is not just the md5, check .md5sum
		if (md5remote.empty())
		{
			endpointMD5 = aEndpointDownload + ".md5sum";
			md5remote = MD5Util::FromRemoteURL(client, endpointMD5);

			// .md5sum also invalid/doesn't exist
			if (md5remote.empty())
			{
				return false;
			}
		}

		// current version is up-to-date
		if (aCurrentMD5 == md5remote)
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
		this->Logger->Warning(CH_UPDATER, "Error fetching %s%s\nError: %s", aBaseURL.c_str(), aEndpointDownload.c_str(), httplib::to_string(downloadResult.error()).c_str());
		
		// try cleaning failed download
		if (std::filesystem::exists(aDownloadPath))
		{
			try
			{
				std::filesystem::remove(aDownloadPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", aDownloadPath.string().c_str());
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
		this->Logger->Warning(CH_UPDATER, "One or more parameters were empty.");
		return false;
	}

	/* prepare client request */
	httplib::Client client(aBaseURL);
	client.enable_server_certificate_verification(true);
	client.set_follow_location(true);

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
		this->Logger->Warning(CH_UPDATER, "Error fetching %s%s\nError: %s", aBaseURL.c_str(), aEndpointDownload.c_str(), httplib::to_string(downloadResult.error()).c_str());
		
		// try cleaning failed download
		if (std::filesystem::exists(aDownloadPath))
		{
			try
			{
				std::filesystem::remove(aDownloadPath);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Warning(CH_UPDATER, "Couldn't remove \"%s\".", aDownloadPath.string().c_str());
			}
		}

		return false;
	}

	return true;
}
