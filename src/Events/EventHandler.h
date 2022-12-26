#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>

typedef void (*ConsumeEventCallback)(void*);									/* void ConsumeEventCallback(void* eventArgs) */
typedef void (*RaiseEventSig)(const wchar_t*, void*);
typedef void (*SubscribeEventSig)(const wchar_t*, ConsumeEventCallback);

namespace EventHandler
{
	void RaiseEvent(const wchar_t*, void*);										/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void SubscribeEvent(const wchar_t*, ConsumeEventCallback);					/* Subscribes the provided ConsumeEventCallback function, to the provided event name */

	static std::mutex EventRegistryMutex;
	static std::map<const wchar_t*, std::vector<ConsumeEventCallback>> EventRegistry;
};

#endif