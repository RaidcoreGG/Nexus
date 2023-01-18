#ifndef LOADER_H
#define LOADER_H

#include "AddonDef.h"
#include <mutex>
#include <map>
#include <filesystem>

namespace Loader
{
	extern std::mutex AddonsMutex;
	extern std::map<std::filesystem::path, AddonDef> AddonDefs;

	void Initialize();

	void Update();
}

#endif