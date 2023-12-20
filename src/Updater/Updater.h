#ifndef UPDATER_H
#define UPDATER_H

#include <filesystem>

#include "Loader/AddonDefinition.h"

namespace Updater
{
	/* Updates Nexus if available. */
	void SelfUpdate();

	/* Returns true if an update for the passed addon is available and was downloaded. */
	bool CheckForUpdate(std::filesystem::path aPath, AddonDefinition* aDefinitions);
}

#endif