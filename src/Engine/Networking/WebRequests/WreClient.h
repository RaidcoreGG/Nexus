///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreClient.h
/// Description  :  Provides functions for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef WRECLIENT_H
#define WRECLIENT_H

#include <string>
#include <cstdint>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <condition_variable>

#include "httplib/httplib.h"

#include "WreCacheEntry.h"
#include "WreRequest.h"
#include "WreResponse.h"
#include "Engine/Logging/LogApi.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// CHttpClient Class
///----------------------------------------------------------------------------------------------------
class CHttpClient
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	/// 	- aCacheDirectory: Directory which will contain the cached requests.
	/// 	- aCacheLifetime: Lifetime of a cache entry in seconds.
	/// 	- aBucketCapacity: Bucket size for requests.
	/// 	- aRefillAmount: How many tokens refill each interval.
	/// 	- aRefillInterval: Bucket refill interval in seconds.
	///----------------------------------------------------------------------------------------------------
	CHttpClient(
		std::string aBaseURL,
		std::filesystem::path aCacheDirectory,
		uint32_t aCacheLifetime,
		uint32_t aBucketCapacity,
		uint32_t aRefillAmount,
		uint32_t aRefillInterval
	);
	
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CHttpClient();

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Sends a http requests and fetches the response.
	/// 	- aOverrideCacheLifetime(seconds) changes the cache lifetime to the given one. -1 means, don't change it.
	///----------------------------------------------------------------------------------------------------
	json Get(std::string aEndpoint, std::string aParameters = "", int32_t aOverrideCacheLifetime = -1);

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
	std::string               BaseURL;
	httplib::Client*          Client;

	std::mutex                Mutex;
	std::filesystem::path     CacheDirectory;
	uint32_t                  CacheLifetime;
	std::unordered_map<
		std::filesystem::path,
		CachedResponse_t*
	>                         ResponseCache;

	uint64_t                  TimeSinceLastRefill;
	uint32_t                  Bucket;
	uint32_t                  BucketCapacity;
	uint32_t                  RefillAmount;
	uint32_t                  RefillInterval;

	std::mutex                ThreadMutex;
	std::thread               WorkerThread;
	std::condition_variable   ConVar;
	bool                      IsSuspended = false;

	std::vector<APIRequest_t> QueuedRequests;

	///----------------------------------------------------------------------------------------------------
	/// GetCachedResponse:
	/// 	Returns a pointer to an existing response or nullptr.
	/// 	- aEndpoint should have a prefixed "/"
	/// 	- aParameters should have a prefixed "?"
	///----------------------------------------------------------------------------------------------------
	CachedResponse_t* GetCachedResponse(const std::string& aQuery);
	
	///----------------------------------------------------------------------------------------------------
	/// GetNormalizedPath:
	/// 	Normalizes a path.
	/// 	- aEndpoint should have a prefixed "/"
	/// 	- aParameters should have a prefixed "?"
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetNormalizedPath(const std::string& aQuery) const;

	private:
	CLogApi*  Logger = nullptr;
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
	APIResponse_t HttpGet(APIRequest_t aRequest);
};

#endif
