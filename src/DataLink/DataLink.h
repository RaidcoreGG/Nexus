#ifndef DATALINK_H
#define DATALINK_H

#include <string>
#include <mutex>
#include <map>

#include "../Consts.h"
#include "../Shared.h"

#include "LinkedResource.h"

namespace DataLink
{
	extern std::mutex								Mutex;
	extern std::map<std::string, LinkedResource>	Registry;

	/* Frees all remaining resources */
	void Free();

	/* Retrieves the resource with the given identifier */
	void* GetResource(std::string aIdentifier);
	/* Allocates new memory of the given size and returns a pointer to it and shares it via the given identifier */
	void* ShareResource(std::string aIdentifier, size_t aResourceSize);
	void* ShareResource(std::string aIdentifier, size_t aResourceSize, std::string aResourceNameOverride);
}

#endif