#ifndef APIHANDLER_H
#define APIHANDLER_H

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

class APIClient
{
public:
	/*
	APIClient:
	- aBaseUrl is the API base
	- aCacheDirectory is the directory to where the requests will be cached on disk
	- aCacheLifetime (seconds) refers to how long a response should still be considered valid, if it's a cached one, before refetching it
	- aBucketCapacity refers to the bucket size for requests
	- aRefillAmount refers to how many tokens you get back after each interval
	- aRefillInterval (seconds) refers to when the bucket gets refilled
	*/
	APIClient(std::string aBaseURL, bool aEnableSSL, std::filesystem::path aCacheDirectory, int aCacheLifetime, int aBucketCapacity, int aRefillAmount, int aRefillInterval);
	~APIClient();

	/*
	Get:
	Returns the response string.
	*/
	json Get(std::string aEndpoint, std::string aParameters = "");
	/*
	Download:
	Downloads the remote resource to disk.
	*/
	void Download(std::filesystem::path aOutPath, std::string aEndpoint, std::string aParameters = "");

protected:
	std::string					BaseURL;
	httplib::Client*			Client;

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

	/*
	GetCachedResponse:
	Returns a pointer to an existing response or nullptr.
	- aEndpoint should have a prefixed "/"
	- aParameters should have a prefixed "?"
	*/
	CachedResponse* GetCachedResponse(const std::string& aQuery);
	/* 
	GetNormalizedPath:
	- aEndpoint should have a prefixed "/"
	- aParameters should have a prefixed "?"
	*/
	std::filesystem::path GetNormalizedPath(const std::string& aQuery) const;

private:
	long long FileTimeOffset;

	void ProcessRequests();
	APIResponse DoHttpReq(APIRequest aRequest);
};

#endif
