#ifndef APICONTROLLER_H
#define APICONTROLLER_H

#include <filesystem>
#include <mutex>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include "../Paths.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

namespace API
{
	extern const char*				BaseURL;

	extern std::mutex				Mutex;
	extern std::vector<std::string> APIKeys;

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
}

#endif