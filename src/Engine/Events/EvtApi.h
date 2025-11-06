///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtApi.h
/// Description  :  Provides functions raise and receive events.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/Loader/Loader.h"
#include "EvtData.h"
#include "EvtFuncDefs.h"

constexpr const char* CH_EVENTS = "Events";

///----------------------------------------------------------------------------------------------------
/// CEventApi Class
///----------------------------------------------------------------------------------------------------
class CEventApi : public virtual IRefCleaner
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CEventApi(CLoader* aLoader);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CEventApi();

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event of provided name, passing a pointer to the payload.
	///----------------------------------------------------------------------------------------------------
	void Raise(const char* aIdentifier, void* aEventData = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event with a payload meant for only a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void Raise(uint32_t aSignature, const char* aIdentifier, void* aEventData = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// Subscribe:
	/// 	Subscribes the provided ConsumeEventCallback function, to the provided event name.
	///----------------------------------------------------------------------------------------------------
	void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

	///----------------------------------------------------------------------------------------------------
	/// Unsubscribe:
	/// 	Unsubscribes the provided ConsumeEventCallback function from the provided event name.
	///----------------------------------------------------------------------------------------------------
	void Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes any elements within the provided address space from the Registry.
	///----------------------------------------------------------------------------------------------------
	int CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::unordered_map<std::string, EventData_t> GetRegistry() const;

	private:
	CLoader*                                     Loader = nullptr;

	mutable std::recursive_mutex                 Mutex;
	std::unordered_map<std::string, EventData_t> Registry;
};
