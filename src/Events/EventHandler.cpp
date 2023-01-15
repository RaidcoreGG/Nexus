#include <thread>

#include "EventHandler.h"
#include "../Shared.h"

namespace EventHandler
{
	void RaiseEvent(const wchar_t* aEventName, void* aEventData)
	{
		Logger->Log(aEventName);

		EventRegistryMutex.lock();

		for (ConsumeEventCallback callback : EventRegistry[aEventName])
		{
			std::thread([callback, aEventData]() { callback(aEventData); }).detach();
		}

		EventRegistryMutex.unlock();
	}

	void SubscribeEvent(const wchar_t* aEventName, ConsumeEventCallback aConsumeEventCallback)
	{
		EventRegistryMutex.lock();

		EventRegistry[aEventName].push_back(aConsumeEventCallback);

		EventRegistryMutex.unlock();
	}
}