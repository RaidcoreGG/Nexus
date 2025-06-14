///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FnRegistry.h
/// Description  :  API for function registry.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef FNREGISTRY_H
#define FNREGISTRY_H

#include <mutex>
#include <unordered_map>
#include <string>

#include "FnEntry.h"
#include "Engine/Logging/LogApi.h"

constexpr const char* CH_FUNCTIONS = "Functions";

///----------------------------------------------------------------------------------------------------
/// CFuncRegistry Class
///----------------------------------------------------------------------------------------------------
class CFuncRegistry
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CFuncRegistry(CLogApi* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CFuncRegistry();

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers a function with the given identifier.
	///----------------------------------------------------------------------------------------------------
	void Register(std::string& aIdentifier, void* aFunction);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters a function with the given identifier.
	///----------------------------------------------------------------------------------------------------
	void Deregister(std::string& aIdentifier, void* aFunction);

	///----------------------------------------------------------------------------------------------------
	/// Query:
	/// 	Queries for a function with the given identifier and returns it or nullptr.
	/// 	If a function is returned, the refcount is incremented.
	///----------------------------------------------------------------------------------------------------
	void* Query(std::string& aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Release:
	/// 	Decrements the refcount of the function with the given identifier.
	///----------------------------------------------------------------------------------------------------
	void Release(std::string& aIdentifier);

	private:
	CLogApi*                                     Logger;

	std::mutex                                   Mutex;
	std::unordered_map<std::string, FuncEntry_t> Registry;
};

#endif
