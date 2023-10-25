#ifndef QUEUEDDADDON_H
#define QUEUEDDADDON_H

#include <filesystem>

#include "ELoaderAction.h"

/* A structure that holds information about a queued addon. */
struct QueuedAddon
{
	ELoaderAction Action;
	std::filesystem::path Path;
};

#endif