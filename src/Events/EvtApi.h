///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtApi.h
/// Description  :  Provides functions raise and receive events.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EVTAPI_H
#define EVTAPI_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "EvtFuncDefs.h"
#include "EvtData.h"
#include "EvtSubscriber.h"

constexpr const char* CH_EVENTS = "Events";

///----------------------------------------------------------------------------------------------------
/// CEventApi Class
///----------------------------------------------------------------------------------------------------
class CEventApi
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event of provided name, passing a pointer to the payload.
	///----------------------------------------------------------------------------------------------------
	void Raise(const char* aIdentifier, void* aEventData = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event with a payload meant for only a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void Raise(signed int aSignature, const char* aIdentifier, void* aEventData = nullptr);

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
	/// Verify:
	/// 	Removes any elements within the provided address space from the Registry.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::unordered_map<std::string, EventData> GetRegistry() const;

	private:
	mutable std::recursive_mutex               Mutex;
	std::unordered_map<std::string, EventData> Registry;
};

#endif
