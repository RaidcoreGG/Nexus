///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RefCleanerContext.cpp
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RefCleanerContext.h"

#include <map>

/*static*/ CRefCleanerContext* CRefCleanerContext::Get()
{
	static CRefCleanerContext s_Context{};
	return &s_Context;
}

void CRefCleanerContext::Register(std::string aComponentName, IRefCleaner* aComponent)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [aComponent](const RefCleaner_t& entry)
	{
		return entry.Component == aComponent;
	});

	/* If already registered. */
	if (it != this->Registry.end())
	{
		return;
	}

	this->Registry.push_back(RefCleaner_t{ aComponentName, aComponent });
}

void CRefCleanerContext::Deregister(IRefCleaner* aComponent)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [aComponent](const RefCleaner_t& entry) {
		return entry.Component == aComponent;
	});

	if (it != this->Registry.end())
	{
		this->Registry.erase(it);
	}
}

std::string CRefCleanerContext::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	std::string result;

	std::map<std::string, uint32_t> leftoverRefs;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (const RefCleaner_t& entry : this->Registry)
	{
		leftoverRefs[entry.Name] += entry.Component->CleanupRefs(aStartAddress, aEndAddress);
	}

	for (auto& [id, count] : leftoverRefs)
	{
#ifndef _DEBUG
		if (count == 0) { continue; }
#endif
		result.append("\t");
		result.append(id);
		result.append(": ");
		result.append(std::to_string(count));
		result.append("\n");
	}

	if (!result.empty())
	{
		result = "Cleaned leftover references for\n" + result;
	}

	return result;
}
