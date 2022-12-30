#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>

typedef void (*ConsumeEventCallback)(void* aEventArgs);
typedef void (*RaiseEventSig)(const wchar_t* aEventName, void* aEventData);
typedef void (*SubscribeEventSig)(const wchar_t* aEventName, ConsumeEventCallback aConsumeEventCallback);

namespace EventHandler
{
	void RaiseEvent(const wchar_t* aEventName, void* aEventData);											/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void SubscribeEvent(const wchar_t* aEventName, ConsumeEventCallback aConsumeEventCallback);				/* Subscribes the provided ConsumeEventCallback function, to the provided event name */

	static std::mutex EventRegistryMutex;
	static std::map<const wchar_t*, std::vector<ConsumeEventCallback>> EventRegistry;
};

#endif