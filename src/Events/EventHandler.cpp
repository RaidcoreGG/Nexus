#include <thread>
#include <algorithm>

#include "EventHandler.h"
#include "../Shared.h"

namespace EventHandler
{
	std::mutex EventRegistryMutex;
	std::map<std::string, std::vector<EVENT_CONSUME>> EventRegistry;

	void RaiseEvent(std::string aIdentifier, void* aEventData)
	{
		Log(aIdentifier.c_str());

		EventRegistryMutex.lock();

		for (EVENT_CONSUME callback : EventRegistry[aIdentifier])
		{
			std::thread([callback, aEventData]() { callback(aEventData); }).detach();
		}

		EventRegistryMutex.unlock();
	}

	void SubscribeEvent(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		EventRegistryMutex.lock();

		EventRegistry[aIdentifier].push_back(aConsumeEventCallback);

		EventRegistryMutex.unlock();
	}

	void UnsubscribeEvent(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		EventRegistryMutex.lock();

		if (EventRegistry.find(aIdentifier) != EventRegistry.end())
		{
			EventRegistry[aIdentifier].erase(std::remove(EventRegistry[aIdentifier].begin(), EventRegistry[aIdentifier].end(), aConsumeEventCallback), EventRegistry[aIdentifier].end());
		}

		EventRegistryMutex.unlock();
	}
}