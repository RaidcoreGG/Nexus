#include <thread>

#include "EventHandler.h"

void EventHandler::RaiseEvent(const wchar_t* aEventName, void* aEventData)
{
	EventRegistryMutex.lock();

	for (ConsumeEventCallback callback : EventRegistry[aEventName])
	{
		std::thread([callback, aEventData]() { callback(aEventData); }).detach();
	}

	EventRegistryMutex.unlock();
}

void EventHandler::SubscribeEvent(const wchar_t* aEventName, ConsumeEventCallback aConsumeEventCallback)
{
	EventRegistryMutex.lock();

	EventRegistry[aEventName].push_back(aConsumeEventCallback);

	EventRegistryMutex.unlock();
}