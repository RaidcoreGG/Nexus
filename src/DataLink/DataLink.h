///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DataLink.h
/// Description  :  Provides functions to share data and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DATALINK_H
#define DATALINK_H

#include <mutex>
#include <unordered_map>
#include <string>

#include "LinkedResource.h"

///----------------------------------------------------------------------------------------------------
/// DataLink Namespace
///----------------------------------------------------------------------------------------------------
namespace DataLink
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_ShareResource:
	/// 	Addon API wrapper function for ShareResource.
	///----------------------------------------------------------------------------------------------------
	void* ADDONAPI_ShareResource(const char* aIdentifier, size_t aResourceSize);
}
///----------------------------------------------------------------------------------------------------
/// DataLink Namespace
///----------------------------------------------------------------------------------------------------
namespace DataLink
{
	extern std::mutex										Mutex;
	extern std::unordered_map<std::string, LinkedResource>	Registry;

	///----------------------------------------------------------------------------------------------------
	/// Free:
	/// 	Frees all remaining resources.
	///----------------------------------------------------------------------------------------------------
	void Free();

	///----------------------------------------------------------------------------------------------------
	/// GetResource:
	/// 	Retrieves the resource with the given identifier.
	///----------------------------------------------------------------------------------------------------
	void* GetResource(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ShareResource:
	/// 	Allocates memory of the given size, accessible via the provided identifier.
	///----------------------------------------------------------------------------------------------------
	void* ShareResource(const char* aIdentifier, size_t aResourceSize);

	///----------------------------------------------------------------------------------------------------
	/// ShareResource:
	/// 	Allocates memory of the given size, accessible via the provided identifier,
	/// 	but with a different internal/underlying name.
	///----------------------------------------------------------------------------------------------------
	void* ShareResource(const char* aIdentifier, size_t aResourceSize, const char* aUnderlyingName);
}

#endif
