///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreClient.h
/// Description  :  Provides functions for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef WRECLIENT_H
#define WRECLIENT_H

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>

#pragma warning(push, 0)
#include "httplib/httplib.h"
#pragma warning(pop)

#include "Engine/Logging/LogApi.h"
#include "WreCache.h"
#include "WreResponse.h"

///----------------------------------------------------------------------------------------------------
/// CHttpClient Class
///----------------------------------------------------------------------------------------------------
class CHttpClient
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	/// 	- aLogger: Logger dependency.
	/// 	- aBaseURL: URL base for the client.
	/// 	- aCacheDirectory: Directory which will contain the cached requests.
	/// 	- aCacheLifetime: Lifetime of a cache entry in seconds.
	///----------------------------------------------------------------------------------------------------
	CHttpClient(
		CLogApi*              aLogger,
		std::string           aBaseURL,
		std::filesystem::path aCacheDirectory = {},
		uint32_t              aCacheLifetime  = 0
	);
	
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CHttpClient();

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Sends a http request and fetches the response.
	/// 	- aOverrideCacheLifetime(seconds) changes the cache lifetime to the given one. -1 means, don't change it.
	///----------------------------------------------------------------------------------------------------
	HttpResponse_t Get(std::string aEndpoint, std::string aParameters = "", int32_t aOverrideCacheLifetime = -1);

	///----------------------------------------------------------------------------------------------------
	/// Download:
	/// 	Downloads a remote resource to disk.
	///----------------------------------------------------------------------------------------------------
	HttpResponse_t Download(std::filesystem::path aOutPath, std::string aEndpoint, std::string aParameters = "");

	private:
	CLogApi*         Logger = nullptr;

	std::string      BaseURL;
	std::mutex       Mutex;
	httplib::Client* Client = nullptr;

	CHttpCache*      Cache  = nullptr;

	///----------------------------------------------------------------------------------------------------
	/// DownloadCleanup:
	/// 	Cleans up a file after a failed download.
	///----------------------------------------------------------------------------------------------------
	void DownloadCleanup(const std::filesystem::path& aOutPath, const std::string& aQuery);
};

#endif
