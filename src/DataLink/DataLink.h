#ifndef DATALINK_H
#define DATALINK_H

#include <string>
#include <mutex>
#include <map>

#include "LinkedResource.h"

typedef void (*DATALINK_GETRESOURCE)(std::string aIdentifier);
typedef void (*DATALINK_SHARERESOURCE)(std::string aIdentifier, int aResourceSize);

namespace DataLink
{
	extern std::mutex DataLinkMutex;
	extern std::map<std::string, LinkedResource> DataLinkRegistry;

	void	Shutdown();														/* Frees all remaining resources */

	void*	GetResource(std::string aIdentifier);							/* Retrieves the resource with the given identifier */
	void*	ShareResource(std::string aIdentifier, size_t aResourceSize);	/* Allocates new memory of the given size and returns a pointer to it and shares with the given identifier */
}

#endif