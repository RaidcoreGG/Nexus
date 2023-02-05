#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <string>

typedef void (*EVENT_CONSUME)(void* aEventArgs);
typedef void (*EVENTS_RAISE)(std::string aEventName, void* aEventData);
typedef void (*EVENTS_SUBSCRIBE)(std::string aEventName, EVENT_CONSUME aConsumeEventCallback);

namespace EventHandler
{
	extern std::mutex EventRegistryMutex;
	extern std::map<std::string, std::vector<EVENT_CONSUME>> EventRegistry;

	void RaiseEvent(std::string aEventName, void* aEventData);										/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void SubscribeEvent(std::string aEventName, EVENT_CONSUME aConsumeEventCallback);				/* Subscribes the provided ConsumeEventCallback function, to the provided event name */
	void UnsubscribeEvent(std::string aEventName, EVENT_CONSUME aConsumeEventCallback);			/* Unsubscribes the provided ConsumeEventCallback function from the provided event name */
};

#endif