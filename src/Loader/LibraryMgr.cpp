///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibraryMgr.cpp
/// Description  :  Loader component for managing addon libraries.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LibraryMgr.h"

#include "httplib/httplib.h"

#include "LoaderConst.h"
#include "Util/src/Url.h"

CLibraryMgr::CLibraryMgr(CLogHandler* aLogger)
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

	for (const std::string& libSrc : this->Sources)
	{
		std::string base     = URL::GetBase(libSrc);
		std::string endpoint = URL::GetEndpoint(libSrc);

		httplib::Client client(base);
		client.set_follow_location(true);
		client.enable_server_certificate_verification(URL::UsingHTTPS(libSrc));

		httplib::Result result = client.Get(endpoint);

		if (!result || result->status != 200)
		{
			this->Logger->Warning(CH_LOADER, "Library: Error fetching \"%s\"\n\tError: %s", libSrc.c_str(), httplib::to_string(result.error()).c_str());
			continue;
		}

		try
		{
			json libJSON = json::parse(result->body);

			if (libJSON.is_null())
			{
				this->Logger->Warning(CH_LOADER, "Library: \"%s\" had an empty response.", libSrc.c_str());
				continue;
			}

			if (!libJSON.is_array())
			{
				this->Logger->Warning(CH_LOADER, "Library: \"%s\" does not conform to library specification.", libSrc.c_str());
				continue;
			}

			for (json& addonJSON : libJSON)
			{
				this->Addons.push_back(addonJSON);
			}
		}
		catch (json::parse_error& ex)
		{
			this->Logger->Warning(CH_LOADER, "Library: \"%s\" could not be parsed.\n\tError: %s", libSrc.c_str(), ex.what());
		}
	}
}

void CLibraryMgr::AddSource(const std::string& aURL)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Sources.push_back(aURL);
}
