#include "Settings.h"

const char* OPT_LASTUISCALE = "LastUIScale";
const char* OPT_FONTSIZE = "FontSize";

namespace Settings
{
	std::mutex	Mutex;
	json		Settings = json::object();

	void Load()
	{
		if (!std::filesystem::exists(Path::F_SETTINGS)) { return; }

		Mutex.lock();
		{
			std::ifstream file(Path::F_SETTINGS);
			Settings = json::parse(file);
			file.close();
		}
		Mutex.unlock();
	}
	void Save()
	{
		Mutex.lock();
		{
			std::ofstream file(Path::F_SETTINGS);
			file << Settings.dump(1, '\t') << std::endl;
			file.close();
		}
		Mutex.unlock();
	}
}