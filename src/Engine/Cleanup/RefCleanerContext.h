///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RefCleanerContext.h
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <mutex>
#include <string>

#include "RefCleanerBase.h"

///----------------------------------------------------------------------------------------------------
/// CRefCleanerContext Class
///----------------------------------------------------------------------------------------------------
class CRefCleanerContext
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns the reference cleaner context.
	///----------------------------------------------------------------------------------------------------
	static CRefCleanerContext* Get();

	CRefCleanerContext(CRefCleanerContext const&) = delete;
	void operator=(CRefCleanerContext const&)     = delete;

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers a reference cleaner with the context.
	///----------------------------------------------------------------------------------------------------
	void Register(std::string aComponentName, IRefCleaner* aComponent);

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes any reference matching the provided address space.
	/// 	Returns a message with the cleanup results.
	///----------------------------------------------------------------------------------------------------
	std::string CleanupRefs(void* aStartAddress, void* aEndAddress);

	private:
	CRefCleanerContext() = default;
	~CRefCleanerContext() = default;

	std::mutex                          Mutex;
	std::map<std::string, IRefCleaner*> Registry;
};
