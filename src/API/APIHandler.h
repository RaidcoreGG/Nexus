#ifndef APIHANDLER_H
#define APIHANLDER_H

#include <string>
#include <mutex>
#include <map>
#include <fstream>
#include <filesystem>

#include "httpslib.h"

#include "CachedResponse.h"

class APIHandler
{
public:
	/*
	APIHandler:
	- aBaseUrl is the API base
	- aCacheDirectory is the directory to where the requests will be cached on disk
	- aCacheLifetime (seconds) refers to how long a response should still be considered valid, if it's a cached one, before refetching it
	- aBucketSize refers to the bucket size for requests
	- aRefillSize refers to how many tokens you get back after each interval
	- aRefillInterval (seconds) refers to when the bucket gets refilled
	*/
	APIHandler(std::string aBaseURL, bool aEnableSSL, std::filesystem::path aCacheDirectory, int aCacheLifetime, int aBucketSize, int aRefillSize, int aRefillInterval);
	~APIHandler();

	const char* Get(std::string aEndpoint, std::string aParameters = "");

protected:
	std::string BaseURL;
	httplib::Client* Client;

	int BucketSize;
	int RefillSize;
	int RefillInterval;

	std::mutex Mutex;
	std::filesystem::path CacheDirectory;
	int CacheLifetime;
	std::map<std::filesystem::path, CachedResponse*> ResponseCache;

	/*
	GetCachedResponse:
	Returns a pointer to an existing response or nullptr.
	- aEndpoint should have a prefixed "/"
	- aParameters should have a prefixed "?"
	*/
	CachedResponse* GetCachedResponse(const std::string& aEndpoint, const std::string& aParameters);
	/* 
	GetNormalizedPath:
	- aEndpoint should have a prefixed "/"
	- aParameters should have a prefixed "?"
	*/
	std::filesystem::path GetNormalizedPath(const std::string& aEndpoint, const std::string& aParameters) const;
};

#endif