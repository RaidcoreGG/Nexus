///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RefCleanerContext.h
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <vector>

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
	/// Deregister:
	/// 	Deregisters a reference cleaner from the context.
	///----------------------------------------------------------------------------------------------------
	void Deregister(IRefCleaner* aComponent);

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes any reference matching the provided address space.
	/// 	Returns a message with the cleanup results.
	///----------------------------------------------------------------------------------------------------
	std::string CleanupRefs(void* aStartAddress, void* aEndAddress);

	private:
	CRefCleanerContext() = default;
	~CRefCleanerContext() = default;

	///----------------------------------------------------------------------------------------------------
	/// RefCleaner_t Struct
	///----------------------------------------------------------------------------------------------------
	struct RefCleaner_t
	{
		std::string  Name;
		IRefCleaner* Component;
	};

	std::mutex                Mutex;
	std::vector<RefCleaner_t> Registry;
};
