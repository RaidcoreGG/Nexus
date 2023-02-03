#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <string>

typedef void (*EVENTS_CONSUME)(void* aEventArgs);
typedef void (*EVENTS_RAISE)(std::wstring aEventName, void* aEventData);
typedef void (*EVENTS_SUBSCRIBE)(std::wstring aEventName, EVENTS_CONSUME aConsumeEventCallback);

namespace EventHandler
{
	static std::mutex EventRegistryMutex;
	static std::map<std::wstring, std::vector<EVENTS_CONSUME>> EventRegistry;

	void RaiseEvent(std::wstring aEventName, void* aEventData);										/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void SubscribeEvent(std::wstring aEventName, EVENTS_CONSUME aConsumeEventCallback);				/* Subscribes the provided ConsumeEventCallback function, to the provided event name */
};

#endif