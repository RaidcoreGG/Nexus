///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreClient.cpp
/// Description  :  Provides functions for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "WreClient.h"

#include <fstream>

#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/NetConst.h"
#include "Util/Paths.h"
#include "Util/Strings.h"
#include "Util/Time.h"
#include "Util/URL.h"

CHttpClient::CHttpClient(CLogApi* aLogger, std::string aBaseURL, std::filesystem::path aCacheDirectory, uint32_t aCacheLifetime, uint32_t aBucketCapacity, uint32_t aRefillAmount, uint32_t aRefillInterval)
{
	this->Logger = aLogger;

	this->BaseURL = URL::GetBase(aBaseURL); // sanitize url just to be sure
	this->Client = new httplib::Client(this->BaseURL);
	this->Client->enable_server_certificate_verification(URL::UsingHTTPS(this->BaseURL));
	this->Client->set_follow_location(true);

	this->CacheDirectory = aCacheDirectory;
	std::filesystem::create_directories(this->CacheDirectory);
	this->CacheLifetime = aCacheLifetime;

	this->TimeSinceLastRefill = Time::GetTimestamp();
	this->Bucket = aBucketCapacity;
	this->BucketCapacity = aBucketCapacity;
	this->RefillAmount = aRefillAmount;
	this->RefillInterval = aRefillInterval;

	this->Logger->Debug(
		CH_NETWORKING,
		"CHttpClient(BaseURL: %s, CacheDirectory: %s, CacheLifetime: %d, BucketCapacity: %d, RefillAmount: %d, RefillInterval: %d)",
		this->BaseURL.c_str(),
		this->CacheDirectory.string().c_str(),
		this->CacheLifetime,
		this->BucketCapacity,
		this->RefillAmount,
		this->RefillInterval
	);

	this->WorkerThread = std::thread(&CHttpClient::ProcessRequests, this);

	std::filesystem::path timeOffset = aCacheDirectory / "0";
	std::ofstream file(timeOffset);
	file << "0" << std::endl;
	file.close();

	this->FileTimeOffset = Time::GetTimestamp() - std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(timeOffset).time_since_epoch()).count();
}
CHttpClient::~CHttpClient()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	while (this->ResponseCache.size() > 0)
	{
		auto it = this->ResponseCache.begin();

		//free((char*)it->second->Content);
		delete it->second;
		this->ResponseCache.erase(it);
	}

	delete this->Client;

	this->Logger->Debug(CH_NETWORKING, "~CHttpClient(%s)", this->BaseURL.c_str());
}

json CHttpClient::Get(std::string aEndpoint, std::string aParameters, int32_t aOverrideCacheLifetime)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	/* override == 0, don't even bother getting the cache */
	CachedResponse_t* cachedResponse = aOverrideCacheLifetime == 0 ? nullptr : GetCachedResponse(query);

	if (cachedResponse != nullptr)
	{
		long long diff = Time::GetTimestamp() - cachedResponse->Timestamp;

		int permittedCacheLifetime = aOverrideCacheLifetime == -1 ? CacheLifetime : aOverrideCacheLifetime;

		if (diff < permittedCacheLifetime && cachedResponse->Content != nullptr)
		{
			//Logger->Debug("CHttpClient", "[%s] Cached message %d seconds old. Reading from cache.", BaseURL.c_str(), diff);
			return cachedResponse->Content;
		}
		else
		{
			//Logger->Debug("CHttpClient", "[%s] Cached message %d seconds old. CacheLifetime %d. Queueing request.", BaseURL.c_str(), diff, CacheLifetime);
		}
	}

	// Variables for synchronization
	std::mutex mtx;
	bool done = false;
	std::condition_variable cv;

	// if not cached, push it into requests queue, so it can be done async
	APIRequest_t req{
		ERequestType::Get,
		&done,
		&cv,
		0,
		query
	};

	// Trigger the worker thread
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		QueuedRequests.push_back(req);
		IsSuspended = false;
		ConVar.notify_all();
	}

	// Wait for the response
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [&] { return done; });
	}

	cachedResponse = nullptr; // sanity
	cachedResponse = GetCachedResponse(query);

	return cachedResponse != nullptr ? cachedResponse->Content : json{};
}
json CHttpClient::Post(std::string aEndpoint, std::string aParameters)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	CachedResponse_t* cachedResponse = GetCachedResponse(query);

	if (cachedResponse != nullptr)
	{
		long long diff = Time::GetTimestamp() - cachedResponse->Timestamp;

		if (diff < CacheLifetime && cachedResponse->Content != nullptr)
		{
			//Logger->Debug("CHttpClient", "[%s] Cached message %d seconds old. Reading from cache.", BaseURL.c_str(), diff);
			return cachedResponse->Content;
		}
		else
		{
			//Logger->Debug("CHttpClient", "[%s] Cached message %d seconds old. CacheLifetime %d. Queueing request.", BaseURL.c_str(), diff, CacheLifetime);
		}
	}

	// Variables for synchronization
	std::mutex mtx;
	bool done = false;
	std::condition_variable cv;

	// if not cached, push it into requests queue, so it can be done async
	APIRequest_t req{
		ERequestType::Post,
		&done,
		&cv,
		0,
		query
	};

	// Trigger the worker thread
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		QueuedRequests.push_back(req);
		IsSuspended = false;
		ConVar.notify_all();
	}

	// Wait for the response
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [&] { return done; });
	}

	cachedResponse = nullptr; // sanity
	cachedResponse = GetCachedResponse(query);

	return cachedResponse != nullptr ? cachedResponse->Content : json{};
}
bool CHttpClient::Download(std::filesystem::path aOutPath, std::string aEndpoint, std::string aParameters)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	size_t bytesWritten = 0;
	std::ofstream file(aOutPath, std::ofstream::binary);
	auto downloadResult = this->Client->Get(query, [&](const char* data, size_t data_length) {
		file.write(data, data_length);
		bytesWritten += data_length;
		return true;
	});
	file.close();

	if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
	{
		this->Logger->Warning(CH_NETWORKING, "[%s] Error fetching %s", this->BaseURL.c_str(), query.c_str());
		return false;
	}

	return true;
}

CachedResponse_t* CHttpClient::GetCachedResponse(const std::string& aQuery)
{
	std::filesystem::path path = GetNormalizedPath(aQuery);

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = this->ResponseCache.find(path);

	if (it != this->ResponseCache.end())
	{
		return it->second;
	}

	if (std::filesystem::exists(path))
	{
		try
		{
			std::ifstream file(path);
			json content = json::parse(file);
			file.close();

			CachedResponse_t* rResponse = new CachedResponse_t{};
			rResponse->Content = content;
			rResponse->Timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(path).time_since_epoch()).count() + this->FileTimeOffset;

			this->ResponseCache.insert({ path, rResponse });
			return rResponse;
		}
		catch (json::parse_error& ex)
		{
			this->Logger->Trace(CH_NETWORKING, "[%s] %s could not be parsed. Error: %s", this->BaseURL.c_str(), path.string().c_str(), ex.what());
		}
	}

	return nullptr;
}
std::filesystem::path CHttpClient::GetNormalizedPath(const std::string& aQuery) const
{
	std::string pathStr = aQuery;

	size_t pos = pathStr.find(".json", pathStr.length() - 5);
	if (pos == std::string::npos)
	{
		// .json not found at end
		pathStr += ".json";
	}

	// replace all illegal chars not permitted in file names: \ / : * ? " < > |
	//pathStr = String::Replace(pathStr, R"(\)", "{bslash}");
	//pathStr = String::Replace(pathStr, R"(/)", "{slash}");
	/*size_t pos = pathStr.find("https://", 0);
	if (pos == std::string::npos)
	{
		// https not found, check for http
		pos = pathStr.find("http://", 0);
		if (pos == std::string::npos)
		{
			// http not found either, replace from idx 0
			pos = 0;
		}
		else
		{
			pos += 7; // length of "http://"
		}
	}
	else
	{
		pos += 8; // length of "https://"
	}*/

	pathStr = String::Replace(pathStr, R"(:)", "{col}");
	pathStr = String::Replace(pathStr, R"(*)", "{ast}");
	pathStr = String::Replace(pathStr, R"(?)", "{qst}");
	pathStr = String::Replace(pathStr, R"(")", "{quot}");
	pathStr = String::Replace(pathStr, R"(<)", "{lt}");
	pathStr = String::Replace(pathStr, R"(>)", "{gt}");
	pathStr = String::Replace(pathStr, R"(|)", "{pipe}");

	return this->CacheDirectory.string() + pathStr.c_str();
}

void CHttpClient::ProcessRequests()
{
	for (;;)
	{
		{
			std::unique_lock<std::mutex> lockThread(this->ThreadMutex);
			this->ConVar.wait(lockThread, [this] { return !this->IsSuspended; });
		}

		const std::lock_guard<std::mutex> lock(this->Mutex);
		/* Do some rate limiting */
		while (this->QueuedRequests.size() > 0)
		{
			/* Calculate current bucket */
			long long deltaRefill = Time::GetTimestamp() - this->TimeSinceLastRefill;
			if (deltaRefill >= this->RefillInterval)
			{
				/* time difference divided by interval gives how many "ticks" happened (rounded down)
				 * multiplied with how many tokens are regenerated
				 * in c++ integer division is truncated toward 0 and is defined behaviour */
				this->Bucket += static_cast<int>((deltaRefill / this->RefillInterval) * this->RefillAmount);

				/* time is not set to current timestamp but rather when the last tick happened*/
				this->TimeSinceLastRefill = Time::GetTimestamp() - (deltaRefill % RefillInterval);

				/* Prevent bucket overflow */
				if (this->Bucket > this->BucketCapacity)
				{
					this->Bucket = this->BucketCapacity;
				}
			}

			/* if bucket empty wait until refill, then start over */
			if (this->Bucket <= 0)
			{
				this->Bucket = 0;

				Sleep(this->RefillInterval - static_cast<int>(deltaRefill) * 1000);
				continue;
			}

			APIRequest_t request = this->QueuedRequests.front();

			// HttpGet should set last request timestamp
			APIResponse_t response = HttpGet(request);

			// does the bucket get reduced on unsuccessful requests? we assume it does
			this->Bucket--;

			bool retry = false;

			/* Refer to: https://en.wikipedia.org/wiki/List_of_HTTP_status_codes */
			if (response.Status >= 200 && response.Status <= 299)
			{
				/* success */
			}
			else if (response.Status >= 300 && response.Status <= 399)
			{
				/* redirect */
			}
			else if (response.Status >= 400 && response.Status <= 499)
			{
				/* client error */
				switch (response.Status)
				{
					case 429: // Rate Limited: explicitly set Bucket to 0 and set TimeSinceLastRefill to now and start over
						this->Bucket = 0;
						this->TimeSinceLastRefill = Time::GetTimestamp();
						retry = true;
						break;
				}
			}
			else if (response.Status >= 500 && response.Status <= 599)
			{
				/* server error */
			}

			/* no retry is considered successful */
			if (!retry)
			{
				CachedResponse_t* cached = new CachedResponse_t{};
				cached->Content = response.Content;
				cached->Timestamp = Time::GetTimestamp();

				std::filesystem::path normalizedPath = GetNormalizedPath(request.Query);

				auto it = this->ResponseCache.find(normalizedPath);

				if (it != this->ResponseCache.end())
				{
					delete it->second;
					this->ResponseCache.erase(normalizedPath);
				}

				this->ResponseCache.insert({ normalizedPath, cached });

				// notify caller
				*(request.IsComplete) = true;
				request.CV->notify_all();

				// write to disk after having notified the caller
				Path::CreateDir(normalizedPath.parent_path());

				std::ofstream file(normalizedPath);
				file << response.Content.dump(1, '\t') << std::endl;
				file.close();
			}
			else
			{
				/* push request to end of queue */
				request.Attempts++;

				/* retry up to 5 times */
				if (request.Attempts < 5)
				{
					this->QueuedRequests.push_back(request);
				}
				else
				{
					*(request.IsComplete) = true;
					request.CV->notify_all();
				}
			}

			/* remove request from queue. either it was handled or pushed to back of queue */
			this->QueuedRequests.erase(this->QueuedRequests.begin());
		}

		// suspend self
		this->IsSuspended = true;
	}
}
APIResponse_t CHttpClient::HttpGet(APIRequest_t aRequest)
{
	APIResponse_t response{
		0,
		nullptr
	};

	httplib::Result result{};
	switch (aRequest.Type)
	{
		case ERequestType::Get:
			result = this->Client->Get(aRequest.Query);
			break;
		case ERequestType::Post:
			result = this->Client->Post(aRequest.Query);
			break;
	}

	if (!result)
	{
		this->Logger->Warning(CH_NETWORKING, "[%s] Error fetching %s | %s", this->BaseURL.c_str(), aRequest.Query.c_str(), httplib::to_string(result.error()).c_str());
		response.Status = 1;
		return response;
	}

	if (result->status != 200) // not HTTP_OK
	{
		this->Logger->Warning(CH_NETWORKING, "[%s] Status %d when fetching %s", this->BaseURL.c_str(), result->status, aRequest.Query.c_str());
		response.Status = result->status;
		return response;
	}

	json jsonResult{};

	try
	{
		jsonResult = json::parse(result->body);
	}
	catch (json::parse_error& ex)
	{
		this->Logger->Warning(CH_NETWORKING, "[%s] Response from %s could not be parsed. Error: %s", this->BaseURL.c_str(), aRequest.Query.c_str(), ex.what());
		return response;
	}

	if (jsonResult.is_null())
	{
		this->Logger->Warning(CH_NETWORKING, "[%s] Error parsing API response from %s.", this->BaseURL.c_str(), aRequest.Query.c_str());
		return response;
	}

	//response.Headers = result->headers;
	/*Logger->Trace("CHttpClient", "[%s] %s", BaseURL.c_str(), aRequest.Query.c_str());
	for (auto it = result->headers.begin(); it != result->headers.end(); ++it)
	{
		if (it->first == "Last-Modified")
		{
			Logger->Trace("meme", "Last-Modified: %d", LastModifiedToTimestamp(it->second));
		}
		Logger->Trace("CHttpClient", "[%s] %s : %s", BaseURL.c_str(), it->first.c_str(), it->second.c_str());
	}*/
	response.Content = jsonResult;
	return response;
}
