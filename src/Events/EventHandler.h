#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>

typedef void (*EVENTS_CONSUME)(void* aEventArgs);
typedef void (*EVENTS_RAISE)(const wchar_t* aEventName, void* aEventData);
typedef void (*EVENTS_SUBSCRIBE)(const wchar_t* aEventName, EVENTS_CONSUME aConsumeEventCallback);

namespace EventHandler
{
	void RaiseEvent(const wchar_t* aEventName, void* aEventData);									/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void SubscribeEvent(const wchar_t* aEventName, EVENTS_CONSUME aConsumeEventCallback);				/* Subscribes the provided ConsumeEventCallback function, to the provided event name */

	static std::mutex EventRegistryMutex;
	static std::map<const wchar_t*, std::vector<EVENTS_CONSUME>> EventRegistry;
};

#endif