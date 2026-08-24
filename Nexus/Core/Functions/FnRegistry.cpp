///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FnRegistry.cpp
/// Description  :  API for function registry.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FnRegistry.h"

namespace Raidcore::Nexus::Core
{
	constexpr const char* LOG_CHANNEL = "Functions";

	FuncRegistry::FuncRegistry(LogApi& aLogger)
		: Logger(aLogger)
	{}

	bool FuncRegistry::Register(const char* aIdentifier, void* aFunction)
	{
		if (aIdentifier == nullptr) { return false; }
		if (aFunction == nullptr) { return false; }

		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it != this->Registry.end())
		{
			this->Logger.Debug(
				LOG_CHANNEL,
				"\"%s\" already registered.",
				aIdentifier
			);
			return false;
		}

		FuncEntry_t entry{};
		entry.RefCount = 0;
		entry.Function = aFunction;

		this->Registry.emplace(aIdentifier, entry);

		return true;
	}

	void FuncRegistry::Deregister(const char* aIdentifier, void* aFunction)
	{
		if (aIdentifier == nullptr) { return; }
		if (aFunction == nullptr) { return; }

		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it == this->Registry.end())
		{
			/* Identifier not registered. */
			return;
		}

		if (it->second.RefCount == 0)
		{
			this->Registry.erase(it);

			this->Logger.Debug(
				LOG_CHANNEL,
				"\"%s\" already at reference count zero. Deleted.",
				aIdentifier
			);
		}
		else
		{
			it->second.ShouldDelete = true;

			this->Logger.Debug(
				LOG_CHANNEL,
				"\"%s\" flagged for deletion. Will be deleted when reaching a reference count of zero.",
				aIdentifier
			);
		}
	}

	void* FuncRegistry::Query(const char* aIdentifier)
	{
		if (aIdentifier == nullptr) { return nullptr; }

		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it != this->Registry.end())
		{
			/* Do not allow new references to function flagged for deletion. */
			if (it->second.ShouldDelete)
			{
				return nullptr;
			}

			it->second.RefCount++;
			return it->second.Function;
		}

		return nullptr;
	}

	void FuncRegistry::Release(const char* aIdentifier)
	{
		if (aIdentifier == nullptr) { return; }

		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aIdentifier);

		if (it != this->Registry.end())
		{
			it->second.RefCount--;

			if (it->second.RefCount < 0)
			{
				this->Logger.Critical(
					LOG_CHANNEL,
					"\"%s\" reference count less than zero. Query/Release mismatch. Function may be freed prematurely.",
					aIdentifier
				);
			}

			if (it->second.RefCount == 0 && it->second.ShouldDelete == true)
			{
				this->Registry.erase(it);

				this->Logger.Debug(
					LOG_CHANNEL,
					"\"%s\" deleted after reaching reference count zero.",
					aIdentifier
				);
			}
		}
	}
}
