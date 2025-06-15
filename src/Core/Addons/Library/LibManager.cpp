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
#include "Engine/Loader/LdrEnum.h"
#include "Util/Strings.h"
#include "Util/Url.h"

CLibraryMgr::CLibraryMgr(CLogApi* aLogger)
{
	this->Logger = aLogger;
}

CLibraryMgr::~CLibraryMgr()
{
	this->Logger = nullptr;
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
			this->Logger->Warning(CH_LIBRARY, "Unknown error processing \"%s\".");
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

	EUpdateProvider provider = GetProvider(aURL);

	CHttpClient* client = nullptr;

	switch (provider)
	{
		case EUpdateProvider::Raidcore:
		{
			CContext* ctx = CContext::GetContext();
			client = ctx->GetRaidcoreApi();
			break;
		}
		case EUpdateProvider::GitHub:
		{
			CContext* ctx = CContext::GetContext();
			client = ctx->GetGitHubApi();
			break;
		}
		default:
		{
			std::string baseUrl = URL::GetBase(aURL);
			baseUrl = String::Replace(baseUrl, "htts://", "");
			baseUrl = String::Replace(baseUrl, "https://", "");

			std::filesystem::path cachedir = Index(EPath::DIR_COMMON) / baseUrl;

			client = new CHttpClient(
				this->Logger,
				aURL,
				cachedir,
				30 * 60 /* 30 minute cache lifetime. */
			);
			break;
		}
	}

	this->Sources.emplace(aURL, client);
}

std::vector<LibraryAddon_t> CLibraryMgr::GetLibrary() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Addons;
}
