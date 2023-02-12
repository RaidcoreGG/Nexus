#ifndef LOADER_H
#define LOADER_H

#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>

#include "AddonDefinition.h"

typedef AddonDefinition*	(*GETADDONDEF)();

struct LoadedAddon
{
	HMODULE Module;
	AddonDefinition* Definitions;
};

namespace Loader
{
	extern std::mutex AddonsMutex;
	extern std::map<std::filesystem::path, LoadedAddon> AddonDefs;
    extern AddonAPI APIDef;

	extern std::thread UpdateThread;

	extern std::set<std::filesystem::path> ExistingLibs;
	extern std::set<std::filesystem::path> Blacklist;

	void Initialize();
	void Shutdown();

	void LoadAddon(std::filesystem::path aPath);
	void UnloadAddon(std::filesystem::path aPath);

	void Update();
}

#endif