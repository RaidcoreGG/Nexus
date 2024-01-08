#include "APIHandler.h"

#include "core.h"
#include "Shared.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

APIHandler::APIHandler(std::string aBaseURL, bool aEnableSSL, std::filesystem::path aCacheDirectory, int aCacheLifetime, int aBucketSize, int aRefillSize, int aRefillInterval)
{
	// sanity
	std::string apibase = GetBaseURL(aBaseURL);
	Client = new httplib::Client(apibase);
	Client->enable_server_certificate_verification(aEnableSSL);

	BaseURL = apibase;
	CacheDirectory = aCacheDirectory;
	CacheLifetime = aCacheLifetime;
	BucketSize = aBucketSize;
	RefillSize = aRefillSize;
	RefillInterval = aRefillInterval;

	LogDebug("APIHandler", "ctor(%s, %s, %s, %d, %d, %d, %d)",
		BaseURL = apibase.c_str(),
		aEnableSSL ? "true" : "false",
		CacheDirectory.c_str(),
		CacheLifetime, BucketSize, RefillSize, RefillInterval);
}

APIHandler::~APIHandler()
{
	const std::lock_guard<std::mutex> lock(Mutex);
	while (ResponseCache.size() > 0)
	{
		auto it = ResponseCache.begin();

		free((char*)it->second->Content);
		delete it->second;
		ResponseCache.erase(it);
	}

	delete Client;

	LogDebug("APIHandler", "dtor(%s)", BaseURL.c_str());
}

const char* APIHandler::Get(std::string aEndpoint, std::string aParameters)
{
	if (!aEndpoint.find("/", 0) == 0)
	{
		aEndpoint = "/" + aEndpoint;
	}
	if (!aParameters.find("?", 0) == 0)
	{
		aParameters = "?" + aParameters;
	}

	CachedResponse* response = GetCachedResponse(aEndpoint, aParameters);

	if (response != nullptr)
	{
		int diff = Timestamp() - response->Timestamp;
		if (diff < CacheLifetime && response->Content != nullptr)
		{
			return response->Content;
		}
	}

	// if not cached, push it into requests queue, so it can be done async
}

CachedResponse* APIHandler::GetCachedResponse(const std::string& aEndpoint, const std::string& aParameters)
{
	std::filesystem::path path = GetNormalizedPath(aEndpoint, aParameters);

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

			std::string str = content.get<std::string>();

			CachedResponse* response = new CachedResponse{};
			response->Content = _strdup(str.c_str());
			response->Timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(path).time_since_epoch()).count();

			ResponseCache.insert({ path, response });
			return response;
		}
		catch (json::parse_error& ex)
		{
			Log("APIHandler", "%s could not be parsed. Error: %s", path.string().c_str(), ex.what());
		}
	}

	return nullptr;
}

std::filesystem::path APIHandler::GetNormalizedPath(const std::string& aEndpoint, const std::string& aParameters) const
{
	std::string pathStr = CacheDirectory.string() + aEndpoint + aParameters + ".json";

	// replace all illegal chars not permitted in file names: \ / : * ? " < > |
	pathStr = String::Replace(pathStr, R"(\)", "{bslash}");
	pathStr = String::Replace(pathStr, R"(/)", "{slash}");
	pathStr = String::Replace(pathStr, R"(:)", "{col}");
	pathStr = String::Replace(pathStr, R"(*)", "{ast}");
	pathStr = String::Replace(pathStr, R"(?)", "{qst}");
	pathStr = String::Replace(pathStr, R"(")", "{quot}");
	pathStr = String::Replace(pathStr, R"(<)", "{lt}");
	pathStr = String::Replace(pathStr, R"(>)", "{gt}");
	pathStr = String::Replace(pathStr, R"(|)", "{pipe}");

	return pathStr;
}