#ifndef UPDATER_H
#define UPDATER_H

#include <filesystem>

#include "Loader/AddonDefinition.h"

namespace Updater
{
	void SelfUpdate();
	bool CheckForUpdate(std::filesystem::path aPath, AddonDefinition* aDefinitions);
}

#endif