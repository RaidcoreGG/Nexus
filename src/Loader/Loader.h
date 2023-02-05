#ifndef LOADER_H
#define LOADER_H

#include "AddonDefinition.h"
#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>

typedef AddonDefinition*	(*GETADDONDEF)();

namespace Loader
{
	extern std::mutex AddonsMutex;
	extern std::map<std::filesystem::path, AddonDefinition*> AddonDefs;
	extern std::map<std::filesystem::path, HMODULE> AddonModules;
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