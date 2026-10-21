///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreCache.cpp
/// Description  :  Cache implementation for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "WreCache.h"

#include <fstream>

#include "Util/Time.h"

CHttpCache::CHttpCache(std::filesystem::path aDirectory, uint32_t aLifetime)
{
	this->Directory = aDirectory;
	std::filesystem::create_directories(aDirectory);
	this->Lifetime = aLifetime;
}

void CHttpCache::Store(std::string aQuery, const HttpResponse_t& aResponse)
{
	if (!aResponse.Success()) { return; }

	auto it = this->Entries.find(aQuery);

	if (it != this->Entries.end())
	{
		it->second = aResponse;
	}
	else
	{
		this->Entries.emplace(aQuery, aResponse);
	}

	std::filesystem::path cachepath = this->GetCachePath(aQuery);
	std::filesystem::create_directories(cachepath.parent_path());
	std::ofstream file(cachepath);
	if (file.is_open())
	{
		json cacheJSON =
		{
			{ "Time",       aResponse.Time       },
			{ "StatusCode", aResponse.StatusCode },
			{ "Error",      aResponse.Error      },
			{ "Content",    aResponse.Content    }
		};

		file << cacheJSON.dump(1, '\t') << std::endl;
		file.close();
	}
}

HttpResponse_t* CHttpCache::Retrieve(std::string aQuery, int32_t aLifetimeOverride)
{
	auto it = this->Entries.find(aQuery);

	long long now = Time::GetTimestamp();
	uint32_t maxAge = aLifetimeOverride > -1 ? aLifetimeOverride : this->Lifetime;

	/* Prepare disk cache path. */
	std::filesystem::path cachepath = this->GetCachePath(aQuery);

	/* Cache entry in memory. */
	if (it != this->Entries.end())
	{
		/* Entry not expired. */
		if (now - it->second.Time < maxAge)
		{
			return &it->second;
		}
		else
		{
			/* Entry is expired, delete accompanying file from disk. */
			if (std::filesystem::exists(cachepath))
			{
				try
				{
					std::filesystem::remove(cachepath);
				}
				catch (...) {}
			}

			/* Remove the entry from memory as well. */
			this->Entries.erase(aQuery);

			return nullptr;
		}
	}

	/* Check if a cache entry is on disk. */
	if (std::filesystem::exists(cachepath))
	{
		std::ifstream file(cachepath);

		try
		{
			json cacheJSON = json::parse(file);
			if (cacheJSON.is_null())
			{
				/* Jump into catch. */
				throw "Json is null.";
			}

			if (!(cacheJSON.contains("Time")
				&& cacheJSON.contains("StatusCode")
				&& cacheJSON.contains("Error")
				&& cacheJSON.contains("Content")))
			{
				/* Jump into catch. */
				throw "Json is invalid format.";
			}

			HttpResponse_t cachedResponse{};
			cacheJSON["Time"].get_to(cachedResponse.Time);
			cacheJSON["StatusCode"].get_to(cachedResponse.StatusCode);
			cacheJSON["Error"].get_to(cachedResponse.Error);
			cacheJSON["Content"].get_to(cachedResponse.Content);

			this->Entries.emplace(aQuery, cachedResponse);

			auto it = this->Entries.find(aQuery);

			if (it != this->Entries.end())
			{
				if (now - it->second.Time < maxAge)
				{
					return &it->second;
				}
			}
		}
		catch (...)
		{
			file.close();

			/* Entry must be invalid, attempt deleting it. */
			try
			{
				std::filesystem::remove(cachepath);
			}
			catch (...) {}
		}
		
		if (file.is_open())
		{
			file.close();
		}
	}

	return nullptr;
}

void CHttpCache::Flush(bool aCleanupOnDisk)
{
	this->Entries.clear();

	if (aCleanupOnDisk)
	{
		/* Remove entire directory tree. */
		std::filesystem::remove_all(this->Directory);

		/* Recreate root directory. */
		std::filesystem::create_directories(this->Directory);
	}
}

std::filesystem::path CHttpCache::GetCachePath(const std::string& aQuery)
{
	return this->Directory / (NormalizeQuery(aQuery) + ".json");
}
