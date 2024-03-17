#include "EventHandler.h"

#include <thread>
#include <algorithm>
#include <chrono>

#include "core.h"
#include "Consts.h"
#include "Shared.h"

#include "Loader/Loader.h"

namespace Events
{
	std::mutex											Mutex;
	std::map<std::string, std::vector<EVENT_CONSUME>>	Registry;

	void Raise(const char* aIdentifier, void* aEventData)
	{
		//auto start_time = std::chrono::high_resolution_clock::now();

		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (EVENT_CONSUME callback : Registry[str])
			{
				callback(aEventData);
			}
		}

		//auto end_time = std::chrono::high_resolution_clock::now();
		//auto time = end_time - start_time;
		//LogDebug(CH_EVENTS, u8"Executed event (%s) in %uµs.", aIdentifier, time / std::chrono::microseconds(1));
	}
	void RaiseNotification(const char* aIdentifier)
	{
		Raise(aIdentifier, nullptr);
	}

	void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			Registry[str].push_back(aConsumeEventCallback);

			/* dirty hack for arcdps (I hate my life) */
			if ((str == "EV_ARCDPS_COMBATEVENT_LOCAL_RAW" || str == "EV_ARCDPS_COMBATEVENT_SQUAD_RAW") && !Loader::IsArcdpsLoaded)
			{
				Loader::DetectArcdps();
			}
		}
	}

	void Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			if (Registry.find(str) != Registry.end())
			{
				Registry[str].erase(std::remove(Registry[str].begin(), Registry[str].end(), aConsumeEventCallback), Registry[str].end());
			}
		}
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		const std::lock_guard<std::mutex> lock(Mutex);
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

		return refCounter;
	}
}