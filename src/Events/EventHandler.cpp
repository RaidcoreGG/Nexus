#include "EventHandler.h"

namespace Events
{
	std::mutex											Mutex;
	std::map<std::string, std::vector<EVENT_CONSUME>>	Registry;

	void Raise(std::string aIdentifier, void* aEventData)
	{
		Log(CH_EVENTS, aIdentifier.c_str());

		Mutex.lock();
		{
			for (EVENT_CONSUME callback : Registry[aIdentifier])
			{
				std::thread([callback, aEventData]() { callback(aEventData); }).detach();
			}
		}
		Mutex.unlock();
	}

	void Subscribe(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		Mutex.lock();
		{
			Registry[aIdentifier].push_back(aConsumeEventCallback);
		}
		Mutex.unlock();
	}

	void Unsubscribe(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		Mutex.lock();
		{
			if (Registry.find(aIdentifier) != Registry.end())
			{
				Registry[aIdentifier].erase(std::remove(Registry[aIdentifier].begin(), Registry[aIdentifier].end(), aConsumeEventCallback), Registry[aIdentifier].end());
			}
		}
		Mutex.unlock();
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Mutex.lock();
		{
			for (auto& [identifier, consumers] : Registry)
			{
				for (EVENT_CONSUME evCb : consumers)
				{
					if (evCb >= aStartAddress && evCb <= aEndAddress)
					{
						consumers.erase(std::remove(consumers.begin(), consumers.end(), evCb), consumers.end());
						refCounter++;
					}
				}
			}
		}
		Mutex.unlock();

		return refCounter;
	}
}