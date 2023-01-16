#ifndef LOADER_H
#define LOADER_H

//#include "AddonDef.h"
#include <mutex>
#include <map>
#include <filesystem>

namespace Loader
{
	static std::mutex AddonsMutex;
	static std::map<std::filesystem::path, int> AddonDefs;

	void Initialize();

	void Update();
}

#endif