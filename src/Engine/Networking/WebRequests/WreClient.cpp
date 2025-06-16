///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreClient.cpp
/// Description  :  Provides functions for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "WreClient.h"

#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/NetConst.h"
#include "Util/URL.h"
#include "Util/Time.h"

CHttpClient::CHttpClient(CLogApi* aLogger, std::string aBaseURL, std::filesystem::path aCacheDirectory, uint32_t aCacheLifetime)
{
	this->Logger = aLogger;

	this->BaseURL = URL::GetBase(aBaseURL); /* Sanitize the URL. */
	this->Client = new httplib::Client(this->BaseURL);
	this->Client->enable_server_certificate_verification(URL::UsingHTTPS(this->BaseURL));
	this->Client->set_follow_location(true);

	this->Logger->Debug(
		CH_NETWORKING,
		"CHttpClient(BaseURL: %s, CacheDirectory: %s, CacheLifetime: %d)",
		this->BaseURL.c_str(),
		aCacheDirectory.empty()
			? "(null)"
			: aCacheDirectory.string().c_str(),
		aCacheLifetime
	);

	/* If caching is enabled. */
	if (!aCacheDirectory.empty() && aCacheLifetime > 0)
	{
		this->Cache = new CHttpCache(aCacheDirectory, aCacheLifetime);
	}
}

CHttpClient::~CHttpClient()
{
	/* If caching was enabled. */
	if (this->Cache)
	{
		delete this->Cache;
		this->Cache = nullptr;
	}

	delete this->Client;
	this->Client = nullptr;

	this->Logger->Debug(CH_NETWORKING, "~CHttpClient(%s)", this->BaseURL.c_str());
}

HttpResponse_t CHttpClient::Get(std::string aEndpoint, std::string aParameters, int32_t aOverrideCacheLifetime)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (this->Cache)
	{
		if (HttpResponse_t* cachedResult = this->Cache->Retrieve(query, aOverrideCacheLifetime))
		{
			this->Logger->Debug(
				CH_NETWORKING,
				"Returning cached result for \"%s\".",
				query.c_str()
			);
			return *cachedResult;
		}
	}

	HttpResponse_t result{};
	result.Time = Time::GetTimestamp();

	httplib::Result getResult = this->Client->Get(query);

	result.StatusCode = getResult->status;

	if (getResult.error() != httplib::Error::Success)
	{
		result.Error = "Lib Error: " + httplib::to_string(getResult.error());
	}

	if (getResult->status >= 400)
	{
		result.Error = StatusCodeToMessage(getResult->status);
	}

	result.Content = getResult->body;
	this->Cache->Store(query, result);

	return result;
}

HttpResponse_t CHttpClient::Download(std::filesystem::path aOutPath, std::string aEndpoint, std::string aParameters)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	const std::lock_guard<std::mutex> lock(this->Mutex);

	HttpResponse_t result{};
	result.Time = Time::GetTimestamp();

	size_t bytesWritten = 0;
	std::ofstream file(aOutPath, std::ofstream::binary);

	if (!file.is_open())
	{
		this->Logger->Warning(
			CH_NETWORKING,
			"[%s] Error downloading from \"%s\" to \"%s\". Could not open file.",
			this->BaseURL.c_str(),
			query.c_str(),
			aOutPath.string().c_str()
		);

		result.Error = "IO Error: Could not open file.";

		return result;
	}

	httplib::Result downloadResult = this->Client->Get(query, [&](const char* data, size_t data_length) {
		file.write(data, data_length);
		bytesWritten += data_length;
		return true;
	});
	file.close();

	bool success = true;

	result.StatusCode = downloadResult->status;

	if (downloadResult.error() != httplib::Error::Success)
	{
		result.Error = "Lib Error: " + httplib::to_string(downloadResult.error());
		success = false;
	}

	if (bytesWritten == 0)
	{
		result.Error = "No bytes were written.";
		success = false;
	}

	size_t contentLength = 0;
	if (downloadResult.value().has_header("Content-Length"))
	{
		contentLength = downloadResult.value().get_header_value_u64("Content-Length");
	}

	if (bytesWritten > 0 && bytesWritten != contentLength)
	{
		result.Error = "Content-Length / Bytes Written mismatch.";
		success = false;
	}

	if (downloadResult->status != 200)
	{
		result.Error = "Non HTTP 200 response.";
		success = false;
	}

	if (!success)
	{
		this->DownloadCleanup(aOutPath, query);
	}

	return result;
}

void CHttpClient::DownloadCleanup(const std::filesystem::path& aOutPath, const std::string& aQuery)
{
	this->Logger->Warning(
		CH_NETWORKING,
		"[%s] Error downloading from \"%s\" to \"%s\".",
		this->BaseURL.c_str(),
		aQuery.c_str(),
		aOutPath.string().c_str()
	);

	try
	{
		if (std::filesystem::exists(aOutPath))
		{
			std::filesystem::remove(aOutPath);
		}
	}
	catch (...)
	{
		this->Logger->Warning(
			CH_NETWORKING,
			"[%s] Subsequently failed deleting \"%s.\" for failed download \"%s\".",
			this->BaseURL.c_str(),
			aOutPath.string().c_str(),
			aQuery.c_str()
		);
	}
}
