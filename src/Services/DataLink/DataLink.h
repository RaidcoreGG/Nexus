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
#include <string>
#include <unordered_map>

#include "LinkedResource.h"
#include "Services/Logging/LogHandler.h"

constexpr const char* CH_DATALINK = "DataLink";

///----------------------------------------------------------------------------------------------------
/// CDataLink Class
///----------------------------------------------------------------------------------------------------
class CDataLink
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CDataLink(CLogHandler* aLogger);
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
	CLogHandler*                                    Logger   = nullptr;

	std::mutex                                      Mutex;
	std::unordered_map<std::string, LinkedResource> Registry;
};

#endif
