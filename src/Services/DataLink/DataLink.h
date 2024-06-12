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

constexpr const char* CH_DATALINK = "DataLink";

///----------------------------------------------------------------------------------------------------
/// DataLink Namespace
///----------------------------------------------------------------------------------------------------
namespace DataLink
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_GetResource:
	/// 	Addon API wrapper function for GetResource.
	///----------------------------------------------------------------------------------------------------
	void* ADDONAPI_GetResource(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_ShareResource:
	/// 	Addon API wrapper function for ShareResource.
	///----------------------------------------------------------------------------------------------------
	void* ADDONAPI_ShareResource(const char* aIdentifier, size_t aResourceSize);
}
///----------------------------------------------------------------------------------------------------
/// CDataLink Class
///----------------------------------------------------------------------------------------------------
class CDataLink
{
public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CDataLink() = default;
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CDataLink();

	///----------------------------------------------------------------------------------------------------
	/// GetResource:
	/// 	Retrieves the resource with the given identifier.
	///----------------------------------------------------------------------------------------------------
	void* GetResource(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ShareResource:
	/// 	Allocates memory of the given size, accessible via the provided identifier.
	///----------------------------------------------------------------------------------------------------
	void* ShareResource(const char* aIdentifier, size_t aResourceSize, bool aIsPublic);

	///----------------------------------------------------------------------------------------------------
	/// ShareResource:
	/// 	Allocates memory of the given size, accessible via the provided identifier,
	/// 	but with a different internal/underlying name.
	///----------------------------------------------------------------------------------------------------
	void* ShareResource(const char* aIdentifier, size_t aResourceSize, const char* aUnderlyingName, bool aIsPublic);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::unordered_map<std::string, LinkedResource> GetRegistry();

private:
	std::mutex										Mutex;
	std::unordered_map<std::string, LinkedResource>	Registry;
};

#endif
