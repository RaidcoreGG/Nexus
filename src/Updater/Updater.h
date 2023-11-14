#ifndef UPDATER_H
#define UPDATER_H

#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>
#include <fstream>

#include "../Consts.h"
#include "../Shared.h"
#include "../Paths.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

namespace Updater
{
	void Initialize();
}

#endif