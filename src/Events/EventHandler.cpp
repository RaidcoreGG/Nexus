#include "EventHandler.h"

namespace Events
{
	std::mutex											Mutex;
	std::map<std::string, std::vector<EVENT_CONSUME>>	Registry;

	void Raise(const char* aIdentifier, void* aEventData)
	{
		std::string str = aIdentifier;

		Log(CH_EVENTS, str.c_str());

		Mutex.lock();
		{
			for (EVENT_CONSUME callback : Registry[str])
			{
				std::thread([callback, aEventData]() { callback(aEventData); }).detach();
			}
		}
		Mutex.unlock();
	}

	void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		std::string str = aIdentifier;

		Mutex.lock();
		{
			Registry[str].push_back(aConsumeEventCallback);
		}
		Mutex.unlock();
	}

	void Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		std::string str = aIdentifier;

		Mutex.lock();
		{
			if (Registry.find(str) != Registry.end())
			{
				Registry[str].erase(std::remove(Registry[str].begin(), Registry[str].end(), aConsumeEventCallback), Registry[str].end());
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