///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RefCleanerContext.cpp
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RefCleanerContext.h"

CRefCleanerContext* CRefCleanerContext::Get()
{
	static CRefCleanerContext s_Context{};
	return &s_Context;
}

void CRefCleanerContext::Register(std::string aComponentName, IRefCleaner* aComponent)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aComponentName);

	/* If already registered. */
	if (it != this->Registry.end())
	{
		return;
	}

	this->Registry.emplace(aComponentName, aComponent);
}

std::string CRefCleanerContext::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	std::string result;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto [id, compontent] : this->Registry)
	{
		uint32_t refs = compontent->CleanupRefs(aStartAddress, aEndAddress);

#ifndef _DEBUG
		if (refs == 0) { continue; }
#endif

		result.append("\t");
		result.append(id);
		result.append(": ");
		result.append(std::to_string(refs));
		result.append("\n");
	}

	if (!result.empty())
	{
		result = "Cleaned leftover references for\n" + result;
	}

	return result;
}
