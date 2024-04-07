///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EventHandler.h
/// Description  :  Provides functions raise and receive events.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <mutex>
#include <map>
#include <vector>
#include <string>

#include "FuncDefs.h"
#include "EventSubscriber.h"

///----------------------------------------------------------------------------------------------------
/// Events Namespace
///----------------------------------------------------------------------------------------------------
namespace Events
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RaiseEvent:
	/// 	Addon API wrapper function for raising events.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RaiseEvent(const char* aIdentifier, void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RaiseNotification:
	/// 	Addon API wrapper function for raising events without payloads.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RaiseNotification(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RaiseEventTargeted:
	/// 	Addon API wrapper function for raising events targeted at a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RaiseEventTargeted(signed int aSignature, const char* aIdentifier, void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_RaiseNotificationTargeted:
	/// 	Addon API wrapper function for raising notifications targeted at a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_RaiseNotificationTargeted(signed int aSignature, const char* aIdentifier);
}

///----------------------------------------------------------------------------------------------------
/// Events Namespace
///----------------------------------------------------------------------------------------------------
namespace Events
{
	extern std::mutex											Mutex;
	extern std::map<std::string, std::vector<EventSubscriber>>	Registry;

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event of provided name, passing a pointer to the payload.
	///----------------------------------------------------------------------------------------------------
	void Raise(const char* aIdentifier, void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event with a payload meant for only a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void Raise(signed int aSignature, const char* aIdentifier, void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event of provided name without a payload.
	///----------------------------------------------------------------------------------------------------
	void Raise(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event without a payload meant for only a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void Raise(signed int aSignature, const char* aIdentifier);

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
}

#endif
