///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibManager.cpp
/// Description  :  Manager for available addon libraries.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LibManager.h"

#include "Core/Context.h"
#include "Core/Index/Index.h"
#include "Core/Addons/AddEnum.h"
#include "Util/Strings.h"
#include "Util/Url.h"
#include "Util/Paths.h"

CLibraryMgr::CLibraryMgr(CLogApi* aLogger, CLoader* aLoader)
{
	this->Logger = aLogger;
	this->Loader = aLoader;
}

CLibraryMgr::~CLibraryMgr()
{
	this->Logger = nullptr;
	this->Loader = nullptr;
}

void CLibraryMgr::Update()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Addons.clear();

	for (auto& [url, client] : this->Sources)
	{
		std::string endpoint = URL::GetEndpoint(url);

		HttpResponse_t result = client->Get(endpoint);

		if (!result.Success())
		{
			this->Logger->Warning(
				CH_LIBRARY,
				"Failed to fetch addon library from \"%s\".\n\tStatus: %s\n\tError: %s",
				url.c_str(),
				result.Status(),
				result.Error.c_str()
			);
			continue;
		}

		try
		{
			json libJSON = result.ContentJSON();

			if (libJSON.is_null())
			{
				this->Logger->Warning(CH_LIBRARY, "\"%s\" had an empty response.", url.c_str());
				continue;
			}

			if (!libJSON.is_array())
			{
				this->Logger->Warning(CH_LIBRARY, "\"%s\" does not conform to library specification.", url.c_str());
				continue;
			}

			for (json& addonJSON : libJSON)
			{
				this->Addons.push_back(addonJSON);
			}
		}
		catch (...)
		{
			this->Logger->Warning(CH_LIBRARY, "Unknown error processing \"%s\".", url.c_str());
		}
	}
}

void CLibraryMgr::AddSource(std::string aURL)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Sources.find(aURL);

	if (it != this->Sources.end())
	{
		this->Logger->Info(CH_LIBRARY, "Source already exists: %s", aURL.c_str());
		return;
	}

	this->Sources.emplace(aURL, CContext::GetContext()->GetHttpClient(aURL));
}

void CLibraryMgr::Install(uint32_t aSignature)
{
	/* Already installed. */
	if (this->Loader->IsTrackedSafe(aSignature)) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find_if(this->Addons.begin(), this->Addons.end(), [aSignature](const LibraryAddon_t& aAddon) {
		return aAddon.Signature == aSignature;
	});

	/* Does not exist in library. */
	if (it == this->Addons.end()) { return; }

	LibraryAddon_t& addon = *it;

	std::string filename;
	
	if (!addon.FriendlyFilename.empty())
	{
		filename = addon.FriendlyFilename;
	}

	std::filesystem::path tmpPath    = Path::GetUnused(Index(EPath::DIR_TEMP) / filename);
	std::filesystem::path targetPath = Path::GetUnused(Index(EPath::DIR_ADDONS) / filename);

	/* If Github repository, resolve direct download url. */
	std::string downloadUrl = addon.DownloadURL;

	if (String::Contains(downloadUrl, "https://github.com"))
	{
		CHttpClient* ghapiclient = CContext::GetContext()->GetHttpClient("https://api.github.com");

		HttpResponse_t result = ghapiclient->Get("/repos" + URL::GetEndpoint(downloadUrl) + "/releases/latest");

		if (!result.Success())
		{
			this->Logger->Warning(
				CH_LIBRARY,
				"Failed to resolve addon download URL for \"%s\".\n\tStatus: %s\n\tError: %s",
				downloadUrl.c_str(),
				result.Status(),
				result.Error.c_str()
			);
			return;
		}

		try
		{
			json releaseJSON = result.ContentJSON();

			if (releaseJSON.is_null())
			{
				this->Logger->Warning(CH_LIBRARY, "\"%s\" had an empty response when resolving.", downloadUrl.c_str());
				return;
			}

			if (releaseJSON["assets"].is_null())
			{
				this->Logger->Warning(CH_LIBRARY, "\"%s\" had no assets when resolving.", downloadUrl.c_str());
				return;
			}

			if (!releaseJSON["assets"].is_array())
			{
				this->Logger->Warning(CH_LIBRARY, "\"%s\" had assets was not an array when resolving.", downloadUrl.c_str());
				return;
			}

			bool found = false;

			for (json& asset : releaseJSON["assets"])
			{
				if (asset.is_null()) { continue; }
				if (asset.is_array()) { continue; } // sanity

				if (asset["name"].is_null()) { continue; }
				if (asset["browser_download_url"].is_null()) { continue; }

				if (!String::EndsWith(asset["name"].get<std::string>(), ".dll")) { continue; }

				if (filename.empty())
				{
					filename = asset["name"];
				}

				downloadUrl = asset["browser_download_url"];
				found = true;
			}

			if (!found)
			{
				this->Logger->Warning(CH_LIBRARY, "No asset found for \"%s\".", downloadUrl.c_str());
				return;
			}
		}
		catch (...)
		{
			this->Logger->Warning(CH_LIBRARY, "Unknown error processing resolution of \"%s\".", downloadUrl.c_str());
		}
	}

	/* Neither friendly filename nor resolved via github. Use normalized addon name. */
	if (filename.empty())
	{
		filename = String::Normalize(addon.Name) + ".dll";
	}

	CHttpClient client = CHttpClient(this->Logger, downloadUrl);

	std::string endpoint = URL::GetEndpoint(downloadUrl);

	HttpResponse_t result = client.Download(tmpPath, endpoint);

	if (!result.Success())
	{
		this->Logger->Warning(
			CH_LIBRARY,
			"Failed to download addon from \"%s\".\n\tStatus: %s\n\tError: %s",
			downloadUrl.c_str(),
			result.Status().empty() ? "(null)" : result.Status().c_str(),
			result.Error.c_str()
		);
		return;
	}

	try
	{
		std::filesystem::rename(tmpPath, targetPath);
	}
	catch (...)
	{
		this->Logger->Warning(
			CH_LIBRARY,
			"Failed to move downloaded addon from \"%s\" to \"%s\".",
			tmpPath.string().c_str(),
			targetPath.string().c_str()
		);
		return;
	}

	this->Logger->Info(
		CH_LIBRARY,
		"Successfully installed \"%s\" (0x%08X) to \"%s\".",
		addon.Name.c_str(),
		addon.Signature,
		targetPath.string().c_str()
	);

	this->Loader->NotifyChanges();
}

std::vector<LibraryAddon_t> CLibraryMgr::GetLibrary() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Addons;
}
