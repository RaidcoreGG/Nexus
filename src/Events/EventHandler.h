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
	extern std::mutex											Mutex;
	extern std::map<std::string, std::vector<EventSubscriber>>	Registry;

	///----------------------------------------------------------------------------------------------------
	/// Raise:
	/// 	Raises an event of provided name, passing a pointer to the payload.
	///----------------------------------------------------------------------------------------------------
	void Raise(const char* aIdentifier, void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// RaiseNotification:
	/// 	Raises an event without payload, a notification.5Raises an event without payload, a notification.
	///----------------------------------------------------------------------------------------------------
	void RaiseNotification(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// RaiseTargeted:
	/// 	Raises an event with a payload meant for only a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void RaiseTargeted(signed int aSignature, const char* aIdentifier, void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// RaiseNotificationTargeted:
	/// 	Raises a notification meant for only a specific subscriber.
	///----------------------------------------------------------------------------------------------------
	void RaiseNotificationTargeted(signed int aSignature, const char* aIdentifier);

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