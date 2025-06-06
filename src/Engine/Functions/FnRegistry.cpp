///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FnRegistry.cpp
/// Description  :  API for function registry.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FnRegistry.h"

#include <assert.h>

CFuncRegistry::CFuncRegistry()
{
}

CFuncRegistry::~CFuncRegistry()
{
}

void CFuncRegistry::Register(std::string& aIdentifier, void* aFunction)
{
	if (aFunction == nullptr) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		auto fbIt = std::find(it->second.Functions.begin(), it->second.Functions.end(), aFunction);

		if (fbIt != it->second.Functions.end())
		{
			/* Function already registered. */
			return;
		}

		it->second.Functions.push_back(aFunction);

		assert(it->second.ActiveFunction);
	}
	else
	{
		FuncEntry_t entry{};
		entry.RefCount = 0;
		entry.Functions.push_back(aFunction);
		entry.ActiveFunction = aFunction;

		this->Registry.emplace(aIdentifier, entry);
	}
}

void CFuncRegistry::Deregister(std::string& aIdentifier, void* aFunction)
{
	if (aFunction == nullptr) { return; }


}

void* CFuncRegistry::Query(std::string& aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		it->second.RefCount++;
		return it->second.ActiveFunction;
	}

	return nullptr;
}

void CFuncRegistry::Release(std::string& aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		it->second.RefCount--;
	}
}
