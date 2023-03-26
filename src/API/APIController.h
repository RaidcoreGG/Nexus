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

	void Initialize();

	void Load();
	void Save();

	void AddKey(std::string aApiKey);
	void RemoveKey(std::string aApiKey);
}

#endif