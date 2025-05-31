///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlApi.h
/// Description  :  Provides functions to share data and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DLAPI_H
#define DLAPI_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "DlLinkedResource.h"
#include "Services/Logging/LogApi.h"

constexpr const char* CH_DATALINK = "DataLink";

///----------------------------------------------------------------------------------------------------
/// CDataLinkApi Class
///----------------------------------------------------------------------------------------------------
class CDataLinkApi
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CDataLinkApi(CLogApi* aLogger);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CDataLinkApi();

	///----------------------------------------------------------------------------------------------------
	/// GetResource:
	/// 	Retrieves the resource with the given identifier.
	///----------------------------------------------------------------------------------------------------
	void* GetResource(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ShareResource:
	/// 	Allocates memory of the given size, accessible via the provided identifier,
	/// 	but with a different internal/underlying name.
	///----------------------------------------------------------------------------------------------------
	void* ShareResource(
		const char* aIdentifier,
		size_t      aResourceSize,
		const char* aUnderlyingName = "",
		bool        aIsPublic       = false
	);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::unordered_map<std::string, LinkedResource> GetRegistry();

	private:
	CLogApi*                                        Logger = nullptr;

	std::mutex                                      Mutex;
	std::unordered_map<std::string, LinkedResource> Registry;
};

#endif
