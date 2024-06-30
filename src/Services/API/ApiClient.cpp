#include "ApiClient.h"

#include <fstream>

#include "Shared.h"

#include "Util/Paths.h"
#include "Util/Strings.h"
#include "Util/Time.h"
#include "Util/URL.h"

CApiClient::CApiClient(std::string aBaseURL, bool aEnableSSL, std::filesystem::path aCacheDirectory, int aCacheLifetime, int aBucketCapacity, int aRefillAmount, int aRefillInterval, const char* aCertificate)
{
	BaseURL = URL::GetBase(aBaseURL); // sanitize url just to be sure
	Client = new httplib::Client(BaseURL);
	if (aCertificate)
	{
		Client->load_ca_cert_store(aCertificate, sizeof(aCertificate));
	}
	Client->enable_server_certificate_verification(aEnableSSL);
	Client->set_follow_location(true);

	CacheDirectory = aCacheDirectory;
	std::filesystem::create_directories(CacheDirectory);
	CacheLifetime = aCacheLifetime;

	TimeSinceLastRefill = Time::GetTimestamp();
	Bucket = aBucketCapacity;
	BucketCapacity = aBucketCapacity;
	RefillAmount = aRefillAmount;
	RefillInterval = aRefillInterval;

	Logger->Debug("CApiClient", "CApiClient(BaseURL: %s, EnableSSL: %s, CacheDirectory: %s, CacheLifetime: %d, BucketCapacity: %d, RefillAmount: %d, RefillInterval: %d)",
		BaseURL.c_str(),
		aEnableSSL ? "true" : "false",
		CacheDirectory.string().c_str(),
		CacheLifetime, BucketCapacity, RefillAmount, RefillInterval);

	WorkerThread = std::thread(&CApiClient::ProcessRequests, this);
	WorkerThread.detach();

	std::filesystem::path timeOffset = aCacheDirectory / "0";
	std::ofstream file(timeOffset);
	file << "0" << std::endl;
	file.close();

	FileTimeOffset = Time::GetTimestamp() - std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(timeOffset).time_since_epoch()).count();
}
CApiClient::~CApiClient()
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

	Logger->Debug("CApiClient", "~CApiClient(%s)", BaseURL.c_str());
}

json CApiClient::Get(std::string aEndpoint, std::string aParameters, bool aBypassCache)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	CachedResponse* cachedResponse = aBypassCache ? nullptr : GetCachedResponse(query);

	if (cachedResponse != nullptr)
	{
		long long diff = Time::GetTimestamp() - cachedResponse->Timestamp;

		if (diff < CacheLifetime && cachedResponse->Content != nullptr)
		{
			//Logger->Debug("CApiClient", "[%s] Cached message %d seconds old. Reading from cache.", BaseURL.c_str(), diff);
			return cachedResponse->Content;
		}
		else
		{
			//Logger->Debug("CApiClient", "[%s] Cached message %d seconds old. CacheLifetime %d. Queueing request.", BaseURL.c_str(), diff, CacheLifetime);
		}
	}

	// Variables for synchronization
	std::mutex mtx;
	bool done = false;
	std::condition_variable cv;

	// if not cached, push it into requests queue, so it can be done async
	APIRequest req{
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
		std::unique_lock lock(mtx);
		cv.wait(lock, [&]{ return done; });
	}

	cachedResponse = nullptr; // sanity
	cachedResponse = GetCachedResponse(query);

	return cachedResponse != nullptr ? cachedResponse->Content : json{};
}
json CApiClient::Post(std::string aEndpoint, std::string aParameters)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	CachedResponse* cachedResponse = GetCachedResponse(query);

	if (cachedResponse != nullptr)
	{
		long long diff = Time::GetTimestamp() - cachedResponse->Timestamp;

		if (diff < CacheLifetime && cachedResponse->Content != nullptr)
		{
			//Logger->Debug("CApiClient", "[%s] Cached message %d seconds old. Reading from cache.", BaseURL.c_str(), diff);
			return cachedResponse->Content;
		}
		else
		{
			//Logger->Debug("CApiClient", "[%s] Cached message %d seconds old. CacheLifetime %d. Queueing request.", BaseURL.c_str(), diff, CacheLifetime);
		}
	}

	// Variables for synchronization
	std::mutex mtx;
	bool done = false;
	std::condition_variable cv;

	// if not cached, push it into requests queue, so it can be done async
	APIRequest req{
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
		std::unique_lock lock(mtx);
		cv.wait(lock, [&] { return done; });
	}

	cachedResponse = nullptr; // sanity
	cachedResponse = GetCachedResponse(query);

	return cachedResponse != nullptr ? cachedResponse->Content : json{};
}
bool CApiClient::Download(std::filesystem::path aOutPath, std::string aEndpoint, std::string aParameters)
{
	std::string query = URL::GetQuery(aEndpoint, aParameters);

	size_t bytesWritten = 0;
	std::ofstream file(aOutPath, std::ofstream::binary);
	auto downloadResult = Client->Get(query, [&](const char* data, size_t data_length) {
		file.write(data, data_length);
		bytesWritten += data_length;
		return true;
		});
	file.close();

	if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
	{
		Logger->Warning("CApiClient", "[%s] Error fetching %s", BaseURL.c_str(), query.c_str());
		return false;
	}
	
	return true;
}

CachedResponse* CApiClient::GetCachedResponse(const std::string& aQuery)
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
			rResponse->Timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(path).time_since_epoch()).count() + FileTimeOffset;

			ResponseCache.insert({ path, rResponse });
			return rResponse;
		}
		catch (json::parse_error& ex)
		{
			Logger->Trace("CApiClient", "[%s] %s could not be parsed. Error: %s", BaseURL.c_str(), path.string().c_str(), ex.what());
		}
	}

	return nullptr;
}
std::filesystem::path CApiClient::GetNormalizedPath(const std::string& aQuery) const
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

void CApiClient::ProcessRequests()
{
	for (;;)
	{
		{
			std::unique_lock lockThread(ThreadMutex);
			ConVar.wait(lockThread, [this] { return !IsSuspended; });
		}

		const std::lock_guard<std::mutex> lock(Mutex);
		/* Do some rate limiting */
		while (QueuedRequests.size() > 0)
		{
			/* Calculate current bucket */
			long long deltaRefill = Time::GetTimestamp() - TimeSinceLastRefill;
			if (deltaRefill >= RefillInterval)
			{
				/* time difference divided by interval gives how many "ticks" happened (rounded down)
				 * multiplied with how many tokens are regenerated
				 * in c++ integer division is truncated toward 0 and is defined behaviour */
				Bucket += static_cast<int>((deltaRefill / RefillInterval) * RefillAmount);

				/* time is not set to current timestamp but rather when the last tick happened*/
				TimeSinceLastRefill = Time::GetTimestamp() - (deltaRefill % RefillInterval);

				/* Prevent bucket overflow */
				if (Bucket > BucketCapacity)
				{
					Bucket = BucketCapacity;
				}
			}

			/* if bucket empty wait until refill, then start over */
			if (Bucket <= 0)
			{
				Bucket = 0;

				Sleep(RefillInterval - deltaRefill * 1000);
				continue;
			}

			APIRequest request = QueuedRequests.front();

			// HttpGet should set last request timestamp
			APIResponse response = HttpGet(request);

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
					TimeSinceLastRefill = Time::GetTimestamp();
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
				CachedResponse* cached = new CachedResponse{};
				cached->Content = response.Content;
				cached->Timestamp = Time::GetTimestamp();

				std::filesystem::path normalizedPath = GetNormalizedPath(request.Query);
				
				auto it = ResponseCache.find(normalizedPath);

				if (it != ResponseCache.end())
				{
					delete it->second;
					ResponseCache.erase(normalizedPath);
				}

				ResponseCache.insert({ normalizedPath, cached });

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
					QueuedRequests.push_back(request);
				}
				else
				{
					*(request.IsComplete) = true;
					request.CV->notify_all();
				}
			}

			/* remove request from queue. either it was handled or pushed to back of queue */
			QueuedRequests.erase(QueuedRequests.begin());
		}

		// suspend self
		IsSuspended = true;
	}
}
APIResponse CApiClient::HttpGet(APIRequest aRequest)
{
	APIResponse response{
		0,
		nullptr
	};

	httplib::Result result{};
	switch (aRequest.Type)
	{
	case ERequestType::Get:
		result = Client->Get(aRequest.Query);
		break;
	case ERequestType::Post:
		result = Client->Post(aRequest.Query);
		break;
	}

	if (!result)
	{
		Logger->Warning("CApiClient", "[%s] Error fetching %s | %s", BaseURL.c_str(), aRequest.Query.c_str(), httplib::to_string(result.error()).c_str());
		response.Status = 1;
		return response;
	}

	if (result->status != 200) // not HTTP_OK
	{
		Logger->Warning("CApiClient", "[%s] Status %d when fetching %s", BaseURL.c_str(), result->status, aRequest.Query.c_str());
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
		Logger->Warning("CApiClient", "[%s] Response from %s could not be parsed. Error: %s", BaseURL.c_str(), aRequest.Query.c_str(), ex.what());
		return response;
	}

	if (jsonResult.is_null())
	{
		Logger->Warning("CApiClient", "[%s] Error parsing API response from %s.", BaseURL.c_str(), aRequest.Query.c_str());
		return response;
	}

	//response.Headers = result->headers;
	/*Logger->Trace("CApiClient", "[%s] %s", BaseURL.c_str(), aRequest.Query.c_str());
	for (auto it = result->headers.begin(); it != result->headers.end(); ++it)
	{
		if (it->first == "Last-Modified")
		{
			Logger->Trace("meme", "Last-Modified: %d", LastModifiedToTimestamp(it->second));
		}
		Logger->Trace("CApiClient", "[%s] %s : %s", BaseURL.c_str(), it->first.c_str(), it->second.c_str());
	}*/
	response.Content = jsonResult;
	return response;
}
