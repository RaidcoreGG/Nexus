#include "APIController.h"

namespace API
{
	const char*					BaseURL = "https://api.guildwars2.com/v2/";

	std::mutex					Mutex;
	std::vector<std::string>	APIKeys;

	void Initialize()
	{
		Load();
	}

	void Load()
	{
		if (!std::filesystem::exists(Path::F_APIKEYS)) { return; }

		Mutex.lock();
		{
			std::ifstream file(Path::F_APIKEYS);
			json j = json::parse(file);
			for (std::string key : j)
			{
				APIKeys.push_back(key);
			}
			file.close();
		}
		Mutex.unlock();
	}
	void Save()
	{
		Mutex.lock();
		{
			json keys = json::array();

			for (std::string key : APIKeys)
			{
				keys.push_back(key);
			}

			std::ofstream file(Path::F_APIKEYS);
			file << keys.dump(1, '\t') << std::endl;
			file.close();
		}
		Mutex.unlock();
	}

	void AddKey(std::string aApiKey)
	{
		Mutex.lock();
		{
			if (std::find(APIKeys.begin(), APIKeys.end(), aApiKey) == APIKeys.end())
			{
				APIKeys.push_back(aApiKey);
			}
		}
		Mutex.unlock();

		Save();
	}
	void RemoveKey(std::string aApiKey)
	{
		Mutex.lock();
		{
			APIKeys.erase(std::find(APIKeys.begin(), APIKeys.end(), aApiKey));
		}
		Mutex.unlock();

		Save();
	}
}