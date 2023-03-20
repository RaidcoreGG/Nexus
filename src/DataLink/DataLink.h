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
	extern std::mutex Mutex;
	extern std::map<std::string, LinkedResource> Registry;

	void	Shutdown();														/* Frees all remaining resources */

	void*	GetResource(std::string aIdentifier);							/* Retrieves the resource with the given identifier */
	void*	ShareResource(std::string aIdentifier, size_t aResourceSize);	/* Allocates new memory of the given size and returns a pointer to it and shares with the given identifier */
}

#endif