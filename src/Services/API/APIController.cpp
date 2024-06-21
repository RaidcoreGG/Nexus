#include "APIController.h"

#include <filesystem>
#include <fstream>
#include <algorithm>

#include "Index.h"
#include "Shared.h"

#include "httplib/httplib.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace API
{
	const char*					BaseURL = "https://api.guildwars2.com/v2/";

	std::mutex					Mutex;
	std::vector<ActiveToken>	Keys;

	std::thread					RequestThread;
	std::vector<std::string>	QueuedRequests;

	void Initialize()
	{
		Load();

		RequestThread = std::thread(ProcessRequestsLoop);
		RequestThread.detach();
	}

	void Load()
	{
		if (!std::filesystem::exists(Index::F_APIKEYS)) { return; }

		API::Mutex.lock();
		{
			std::ifstream file(Index::F_APIKEYS);
			json j = json::parse(file);
			for (std::string key : j)
			{
				ActiveToken token{};
				token.Key = key;
				Keys.push_back(token);
			}
			file.close();
		}
		API::Mutex.unlock();
	}
	void Save()
	{
		API::Mutex.lock();
		{
			json keys = json::array();

			for (ActiveToken token : Keys)
			{
				keys.push_back(token.Key);
			}

			std::ofstream file(Index::F_APIKEYS);
			file << keys.dump(1, '\t') << std::endl;
			file.close();
		}
		API::Mutex.unlock();
	}

	void AddKey(std::string aApiKey)
	{
		ActiveToken token{};
		token.Key = aApiKey;

		API::Mutex.lock();
		{
			if (std::find(Keys.begin(), Keys.end(), token) == Keys.end())
			{
				Keys.push_back(token);
			}
		}
		API::Mutex.unlock();

		Save();
	}
	void RemoveKey(std::string aApiKey)
	{
		ActiveToken token{};
		token.Key = aApiKey;

		API::Mutex.lock();
		{
			Keys.erase(std::find(Keys.begin(), Keys.end(), token));
		}
		API::Mutex.unlock();

		Save();
	}

	std::string Request(std::string aEndpoint, std::string aParameters)
	{
		Logger->Debug("Fetching %s/%s%s", BaseURL, aEndpoint, aParameters);

		/* Check cache first */

		/* If not cached, request from API */

		/* Cache result */

		return "";
	}
	std::string RequestAuthByToken(std::string aEndpoint, std::string aParameters, ActiveToken aApiKey)
	{
		/* append token to parameters */
		std::string params = aParameters;
		params.append(params.length() == 0 ? "?" : "&");
		params.append("access_token=");
		params.append(aApiKey.Key);

		return Request(aEndpoint, params);
	}
	std::string RequestAuthByAccount(std::string aEndpoint, std::string aParameters, std::string aAccountName)
	{
		/* find token via account name */
		ActiveToken key{};

		API::Mutex.lock();
		{
			for (ActiveToken token : Keys)
			{
				if (token.AccountName == aAccountName)
				{
					key = token;
					break;
				}
			}
		}
		API::Mutex.unlock();

		if (key == ActiveToken{})
		{
			json j;
			j["error"] = "No API-Key matches the provided account name.";

			return j.dump();
		}
		
		return RequestAuthByToken(aEndpoint, aParameters, key);
	}
	std::string RequestAuthByCharacter(std::string aEndpoint, std::string aParameters, std::string aCharacterName)
	{
		/* find token via character name */
		ActiveToken key{};

		API::Mutex.lock();
		{
			for (ActiveToken token : Keys)
			{
				if (std::find(token.Characters.begin(), token.Characters.end(), aCharacterName) != token.Characters.end())
				{
					key = token;
					break;
				}
			}
		}
		API::Mutex.unlock();

		if (key == ActiveToken{})
		{
			json j;
			j["error"] = "No API-Key matches the provided character name.";

			return j.dump();
		}

		return RequestAuthByToken(aEndpoint, aParameters, key);
	}

	void ProcessRequestsLoop()
	{
		for (;;)
		{
			/* Do some rate limiting */
			while (QueuedRequests.size() > 0)
			{
				//HttpGet(QueuedRequests.front());
				/* Callback ? */
				QueuedRequests.erase(QueuedRequests.begin());
			}
		}
	}
}