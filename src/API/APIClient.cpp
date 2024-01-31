#include "APIClient.h"

#include <fstream>

#include "core.h"
#include "Shared.h"

APIClient::APIClient(std::string aBaseURL, bool aEnableSSL, std::filesystem::path aCacheDirectory, int aCacheLifetime, int aBucketCapacity, int aRefillAmount, int aRefillInterval)
{
	BaseURL = GetBaseURL(aBaseURL); // sanitize url just to be sure
	Client = new httplib::Client(BaseURL);
	Client->enable_server_certificate_verification(aEnableSSL);
	Client->set_follow_location(true);

	CacheDirectory = aCacheDirectory;
	std::filesystem::create_directories(CacheDirectory);
	CacheLifetime = aCacheLifetime;

	TimeSinceLastRefill = 0;
	Bucket = 0;
	BucketCapacity = aBucketCapacity;
	RefillAmount = aRefillAmount;
	RefillInterval = aRefillInterval;

	LogDebug("APIClient", "APIClient(BaseURL: %s, EnableSSL: %s, CacheDirectory: %s, CacheLifetime: %d, BucketCapacity: %d, RefillAmount: %d, RefillInterval: %d)",
		BaseURL.c_str(),
		aEnableSSL ? "true" : "false",
		CacheDirectory.string().c_str(),
		CacheLifetime, BucketCapacity, RefillAmount, RefillInterval);
}
APIClient::~APIClient()
{
	const std::lock_guard<std::mutex> lock(Mutex);
	while (ResponseCache.size() > 0)
	{
		auto it = ResponseCache.begin();

		//free((char*)it->second->Content);
		delete it->second;
		ResponseCache.erase(it);
	}

	delete Client;

	LogDebug("APIClient", "~APIClient(%s)", BaseURL.c_str());
}

json APIClient::Get(std::string aEndpoint, std::string aParameters)
{
	std::string query = GetQuery(aEndpoint, aParameters);

	CachedResponse* cachedResponse = GetCachedResponse(query);

	if (cachedResponse != nullptr)
	{
		long long diff = Timestamp() - cachedResponse->Timestamp;
		Log("APIClient", "diff: %d | curr: %d | cache: %d", diff, Timestamp(), cachedResponse->Timestamp);

		if (diff < CacheLifetime && cachedResponse->Content != nullptr)
		{
			return cachedResponse->Content;
		}
	}

	APIRequest request{
		nullptr,
		0,
		query
	};

	// DoHttpReq should set last request timestamp
	APIResponse response = DoHttpReq(request);

	// does the bucket get reduced on unsuccessful requests? we assume it does
	Bucket--;

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
			Bucket = 0;
			TimeSinceLastRefill = Timestamp();
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
		return response.Content;
	}
	else
	{
		/* push request to end of queue */
		request.Attempts++;

		/* retry up to 5 times */
		if (request.Attempts < 5)
		{
			QueuedRequests.push_back(request);
		}
		else
		{
			return nullptr;
		}
	}
}
void APIClient::GetAsync(API_RESPONSE_CALLBACK aCallback, std::string aEndpoint, std::string aParameters)
{
	std::string query = GetQuery(aEndpoint, aParameters);

	CachedResponse* cachedResponse = GetCachedResponse(query);

	if (cachedResponse != nullptr)
	{
		int diff = Timestamp() - cachedResponse->Timestamp;
		if (diff < CacheLifetime && cachedResponse->Content != nullptr)
		{
			if (aCallback != nullptr)
			{
				aCallback(cachedResponse->Content);
			}
			return;
		}
	}

	// if not cached, push it into requests queue, so it can be done async
	APIRequest req{
		aCallback,
		0,
		query
	};

	const std::lock_guard<std::mutex> lock(Mutex);
	QueuedRequests.push_back(req);
	ConVar.notify_one();
}

CachedResponse* APIClient::GetCachedResponse(const std::string& aQuery)
{
	std::filesystem::path path = GetNormalizedPath(aQuery);

	const std::lock_guard<std::mutex> lock(Mutex);
	auto it = ResponseCache.find(path);

	if (it != ResponseCache.end())
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

			CachedResponse* rResponse = new CachedResponse{};
			rResponse->Content = content;
			rResponse->Timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(path).time_since_epoch()).count();

			ResponseCache.insert({ path, rResponse });
			return rResponse;
		}
		catch (json::parse_error& ex)
		{
			Log("APIClient", "%s could not be parsed. Error: %s", path.string().c_str(), ex.what());
		}
	}

	return nullptr;
}
std::filesystem::path APIClient::GetNormalizedPath(const std::string& aQuery) const
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

	return CacheDirectory.string() + pathStr.c_str();
}

void APIClient::ProcessRequests()
{
	for (;;)
	{
		/* FIXME: Apply this same logic in the loader. */
		std::unique_lock<std::mutex> lockThread(ThreadMutex);
		while (IsSuspended)
		{
			ConVar.wait(lockThread);
		}
		lockThread.unlock();

		const std::lock_guard<std::mutex> lock(Mutex);
		/* Do some rate limiting */
		while (QueuedRequests.size() > 0)
		{
			/* Calculate current bucket */
			int deltaRefill = Timestamp() - TimeSinceLastRefill;
			if (deltaRefill > RefillInterval)
			{
				/* time difference divided by interval gives how many "ticks" happened (rounded down)
				 * multiplied with how many tokens are regenerated
				 * in c++ integer division is truncated toward 0 and is defined behaviour */
				Bucket += (deltaRefill / RefillInterval) * RefillAmount;

				/* time is not set to current timestamp but rather when the last tick happened*/
				TimeSinceLastRefill = Timestamp() - (deltaRefill % RefillInterval);

				/* Prevent bucket overflow */
				if (Bucket > BucketCapacity)
				{
					Bucket = BucketCapacity;
				}
			}

			/* if bucket empty wait until refill, then start over */
			if (Bucket <= 0)
			{
				Sleep((RefillInterval - deltaRefill) * 1000);
				continue;
			}

			APIRequest request = QueuedRequests.front();

			// DoHttpReq should set last request timestamp
			APIResponse response = DoHttpReq(request);

			// does the bucket get reduced on unsuccessful requests? we assume it does
			Bucket--;

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
					Bucket = 0;
					TimeSinceLastRefill = Timestamp();
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
				request.Callback(response.Content);
			}
			else
			{
				/* push request to end of queue */
				request.Attempts++;

				/* retry up to 5 times */
				if (request.Attempts < 5)
				{
					QueuedRequests.push_back(request);
				}
				else
				{
					request.Callback(nullptr);
				}
			}

			/* remove request from queue. either it was handled or pushed to back of queue */
			QueuedRequests.erase(QueuedRequests.begin());
		}

		// suspend self
		IsSuspended = true;
	}
}
APIResponse APIClient::DoHttpReq(APIRequest aRequest)
{
	APIResponse response{
		0,
		nullptr
	};

	auto result = Client->Get(aRequest.Query);

	if (!result)
	{
		LogWarning("APIClient", "Error fetching %s", aRequest.Query.c_str());
		response.Status = 1;
		return response;
	}

	if (result->status != 200) // not HTTP_OK
	{
		LogWarning("APIClient", "Status %d when fetching %s", result->status, aRequest.Query.c_str());
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
		LogWarning("APIClient", "Response from %s could not be parsed. Error: %s", aRequest.Query.c_str(), ex.what());
		return response;
	}

	if (jsonResult.is_null())
	{
		LogWarning("APIClient", "Error parsing API response from %s.", aRequest.Query.c_str());
		return response;
	}

	CachedResponse* cached = new CachedResponse{};
	cached->Content = jsonResult;
	cached->Timestamp = Timestamp();

	const std::lock_guard<std::mutex> lock(Mutex);
	std::filesystem::path normalizedPath = GetNormalizedPath(aRequest.Query);

	ResponseCache.insert({ normalizedPath, cached});

	std::ofstream file(normalizedPath);
	file << jsonResult.dump(1, '\t') << std::endl;
	file.close();

	response.Content = jsonResult;
	return response;
}
