#include "Shared.h"

#include "Version.h"

CApiClient*					RaidcoreAPI			= nullptr;
CApiClient*					GitHubAPI			= nullptr;

std::string					ChangelogText;
bool						IsUpdateAvailable	= false;

bool						IsGameLaunchSequence = true;
