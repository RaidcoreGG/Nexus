#include <thread>

#include "EventHandler.h"
#include "../Shared.h"

namespace EventHandler
{
	void RaiseEvent(std::wstring aEventName, void* aEventData)
	{
		Log(aEventName.c_str());

		EventRegistryMutex.lock();

		for (EVENTS_CONSUME callback : EventRegistry[aEventName])
		{
			std::thread([callback, aEventData]() { callback(aEventData); }).detach();
		}

		EventRegistryMutex.unlock();
	}

	void SubscribeEvent(std::wstring aEventName, EVENTS_CONSUME aConsumeEventCallback)
	{
		EventRegistryMutex.lock();

		EventRegistry[aEventName].push_back(aConsumeEventCallback);

		EventRegistryMutex.unlock();
	}
}