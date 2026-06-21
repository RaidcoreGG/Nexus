///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreCache.h
/// Description  :  Cache implementation for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <filesystem>
#include <cstdint>
#include <unordered_map>

#include "WreResponse.h"

///----------------------------------------------------------------------------------------------------
/// CHttpCache Class
///----------------------------------------------------------------------------------------------------
class CHttpCache
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	/// 	- aDirectory: Directory which will contain the cached requests.
	/// 	- aLifetime: Lifetime of a cache entry in seconds.
	///----------------------------------------------------------------------------------------------------
	CHttpCache(std::filesystem::path aDirectory, uint32_t aLifetime);

	///----------------------------------------------------------------------------------------------------
	/// Store:
	/// 	Stores a web request response on disk and runtime cache, if it was successful.
	///----------------------------------------------------------------------------------------------------
	void Store(std::string aQuery, const HttpResponse_t& aResponse);

	///----------------------------------------------------------------------------------------------------
	/// Retrieve:
	/// 	Retrieves a web request response, if it exists and has not expired.
	/// 	aLifetimeOverride: -1 keep default lifetime. 0 >= use parameter lifetime.
	///----------------------------------------------------------------------------------------------------
	HttpResponse_t* Retrieve(std::string aQuery, int32_t aLifetimeOverride = -1);

	///----------------------------------------------------------------------------------------------------
	/// Flush:
	/// 	Flushes the cache. If specified also deletes the cache on disk.
	///----------------------------------------------------------------------------------------------------
	void Flush(bool aCleanupOnDisk = false);

	private:
	std::mutex                                      Mutex;
	std::filesystem::path                           Directory;
	uint32_t                                        Lifetime = 300;
	std::unordered_map<std::string, HttpResponse_t> Entries;

	///----------------------------------------------------------------------------------------------------
	/// GetCachePath:
	/// 	Builds the full cache entry path given a query.
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetCachePath(const std::string& aQuery);
};
