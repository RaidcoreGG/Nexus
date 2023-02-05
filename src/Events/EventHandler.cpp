#include <thread>
#include <algorithm>

#include "EventHandler.h"
#include "../Shared.h"

namespace EventHandler
{
	std::mutex EventRegistryMutex;
	std::map<std::string, std::vector<EVENT_CONSUME>> EventRegistry;

	void RaiseEvent(std::string aEventName, void* aEventData)
	{
		Log(aEventName.c_str());

		EventRegistryMutex.lock();

		for (EVENT_CONSUME callback : EventRegistry[aEventName])
		{
			std::thread([callback, aEventData]() { callback(aEventData); }).detach();
		}

		EventRegistryMutex.unlock();
	}

	void SubscribeEvent(std::string aEventName, EVENT_CONSUME aConsumeEventCallback)
	{
		EventRegistryMutex.lock();

		EventRegistry[aEventName].push_back(aConsumeEventCallback);

		EventRegistryMutex.unlock();
	}

	void UnsubscribeEvent(std::string aEventName, EVENT_CONSUME aConsumeEventCallback)
	{
		EventRegistryMutex.lock();

		if (EventRegistry.find(aEventName) != EventRegistry.end())
		{
			EventRegistry[aEventName].erase(std::remove(EventRegistry[aEventName].begin(), EventRegistry[aEventName].end(), aConsumeEventCallback), EventRegistry[aEventName].end());
		}

		EventRegistryMutex.unlock();
	}
}