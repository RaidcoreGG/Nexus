///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ApiClient.h
/// Description  :  Provides functions for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef APICLIENT_H
#define APICLIENT_H

#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <condition_variable>

#include "httplib/httplib.h"

#include "CachedResponse.h"
#include "ApiRequest.h"
#include "ApiResponse.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// CApiClient Class
///----------------------------------------------------------------------------------------------------
class CApiClient
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor:
	/// 	- aCacheDirectory is the directory to where the requests will be cached on disk
	/// 	- aCacheLifetime(seconds) refers to how long a response should still be considered valid, if it's a cached one, before refetching it
	/// 	- aBucketCapacity refers to the bucket size for requests
	/// 	- aRefillAmount refers to how many tokens you get back after each interval
	/// 	- aRefillInterval(seconds) refers to when the bucket gets refilled
	///----------------------------------------------------------------------------------------------------
	CApiClient(std::string aBaseURL, bool aEnableSSL, std::filesystem::path aCacheDirectory, int aCacheLifetime, int aBucketCapacity, int aRefillAmount, int aRefillInterval, const char* aCertificate = nullptr);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CApiClient();

	/*
	Get:
	Returns the response string.
	*/

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Sends a http requests and fetches the response.
	/// 	- aOverrideCacheLifetime(seconds) changes the cache lifetime to the given one. -1 means, don't change it.
	///----------------------------------------------------------------------------------------------------
	json Get(std::string aEndpoint, std::string aParameters = "", int aOverrideCacheLifetime = -1);

	///----------------------------------------------------------------------------------------------------
	/// Post:
	/// 	Sends a http post.
	///----------------------------------------------------------------------------------------------------
	json Post(std::string aEndpoint, std::string aParameters = "");

	///----------------------------------------------------------------------------------------------------
	/// Download:
	/// 	Downloads a remote resource to disk.
	///----------------------------------------------------------------------------------------------------
	bool Download(std::filesystem::path aOutPath, std::string aEndpoint, std::string aParameters = "");

	protected:
	std::string					BaseURL;
	httplib::Client* Client;

	std::mutex					Mutex;
	std::filesystem::path		CacheDirectory;
	int							CacheLifetime;
	std::unordered_map<
		std::filesystem::path,
		CachedResponse*
	>							ResponseCache;

	long long					TimeSinceLastRefill;
	int							Bucket;
	int							BucketCapacity;
	int							RefillAmount;
	int							RefillInterval;

	std::mutex					ThreadMutex;
	std::thread					WorkerThread;
	std::condition_variable		ConVar;
	bool						IsSuspended = false;

	std::vector<APIRequest>		QueuedRequests;

	///----------------------------------------------------------------------------------------------------
	/// GetCachedResponse:
	/// 	Returns a pointer to an existing response or nullptr.
	/// 	- aEndpoint should have a prefixed "/"
	/// 	- aParameters should have a prefixed "?"
	///----------------------------------------------------------------------------------------------------
	CachedResponse* GetCachedResponse(const std::string& aQuery);
	
	///----------------------------------------------------------------------------------------------------
	/// GetNormalizedPath:
	/// 	Normalizes a path.
	/// 	- aEndpoint should have a prefixed "/"
	/// 	- aParameters should have a prefixed "?"
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetNormalizedPath(const std::string& aQuery) const;

	private:
	long long FileTimeOffset;

	///----------------------------------------------------------------------------------------------------
	/// ProcessRequests:
	/// 	Loop to handle http requests.
	///----------------------------------------------------------------------------------------------------
	void ProcessRequests();

	///----------------------------------------------------------------------------------------------------
	/// HttpGet:
	/// 	Internal HTTP get.
	///----------------------------------------------------------------------------------------------------
	APIResponse HttpGet(APIRequest aRequest);
};

#endif
