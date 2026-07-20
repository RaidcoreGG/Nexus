///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FnRegistry.cpp
/// Description  :  API for function registry.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FnRegistry.h"

#include <assert.h>

namespace Raidcore::Nexus::Core
{
	FuncRegistry::FuncRegistry(LogApi* aLogger)
	{
		assert(aLogger);

		this->Logger = aLogger;
	}

	FuncRegistry::~FuncRegistry()
	{}

	void FuncRegistry::Register(std::string& aIdentifier, void* aFunction)
	{
		if (aFunction == nullptr) { return; }

		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it != this->Registry.end())
		{
			/* Identifier already registered. */
			return;
		}

		FuncEntry_t entry{};
		entry.RefCount = 0;
		entry.Function = aFunction;

		this->Registry.emplace(aIdentifier, entry);
	}

	void FuncRegistry::Deregister(std::string& aIdentifier, void* aFunction)
	{
		if (aFunction == nullptr) { return; }


	}

	void* FuncRegistry::Query(std::string& aIdentifier)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it != this->Registry.end())
		{
			it->second.RefCount++;
			return it->second.Function;
		}

		return nullptr;
	}

	void FuncRegistry::Release(std::string& aIdentifier)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it != this->Registry.end())
		{
			it->second.RefCount--;

			if (it->second.RefCount < 0)
			{
				this->Logger->Critical(
					CH_FUNCTIONS,
					"%s reference count less than zero. Query/Release mismatch. Function may be freed prematurely.",
					aIdentifier.c_str()
				);
			}
		}
	}
}
