#ifndef APICONTROLLER_H
#define APICONTROLLER_H

#include <filesystem>
#include <mutex>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>

#include "../Paths.h"
#include "../Shared.h"

#include "ActiveToken.h"

#include "../httplib/httplib.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

namespace API
{
	extern const char*					BaseURL;

	extern std::mutex					Mutex;
	extern std::vector<ActiveToken>		Keys;

	extern std::thread					RequestThread;
	extern std::vector<std::string>		QueuedRequests;

	/* Initialize the APIController. */
	void Initialize();

	/* Load the API keys. */
	void Load();
	/* Save the API keys.*/
	void Save();

	/* Add a new API key & save. */
	void AddKey(std::string aApiKey);
	/* Remove an API key & save. */
	void RemoveKey(std::string aApiKey);

	std::string RequestAuthByToken(std::string aEndpoint, std::string aParameters, ActiveToken aApiKey);
	std::string RequestAuthByAccount(std::string aEndpoint, std::string aParameters, std::string aAccountName);
	std::string RequestAuthByCharacter(std::string aEndpoint, std::string aParameters, std::string aCharacterName);

	/* Loops and processes active API requests. */
	void ProcessRequestsLoop();
}

#endif