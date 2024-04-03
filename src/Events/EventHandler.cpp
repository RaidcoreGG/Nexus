///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EventHandler.cpp
/// Description  :  Provides functions raise and receive events.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

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
	std::map<std::string, std::vector<EventSubscriber>>	Registry;

	void Raise(const char* aIdentifier, void* aEventData)
	{
		//auto start_time = std::chrono::high_resolution_clock::now();

		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (EventSubscriber sub : Registry[str])
			{
				sub.Callback(aEventData);
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

	void RaiseTargeted(signed int aSignature, const char* aIdentifier, void* aEventData)
	{
		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (EventSubscriber sub : Registry[str])
			{
				if (sub.Signature == aSignature)
				{
					sub.Callback(aEventData);
					break;
				}
			}
		}
	}

	void RaiseNotificationTargeted(signed int aSignature, const char* aIdentifier)
	{
		RaiseTargeted(aSignature, aIdentifier, nullptr);
	}

	void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			EventSubscriber sub{};
			sub.Callback = aConsumeEventCallback;

			for (auto& [path, addon] : Loader::Addons)
			{
				if (addon->Module == nullptr || 
					addon->ModuleSize == 0 ||
					addon->Definitions == nullptr || 
					addon->Definitions->Signature == 0)
				{
					continue;
				}

				void* startAddress = addon->Module;
				void* endAddress = ((PBYTE)addon->Module) + addon->ModuleSize;

				if (aConsumeEventCallback >= startAddress && aConsumeEventCallback <= endAddress)
				{
					sub.Signature = addon->Definitions->Signature;
					break;
				}
			}

			Registry[str].push_back(sub);

			if (sub.Signature == 0)
			{
				LogWarning(CH_EVENTS, "Event registered but no addon address space matches function pointer. %p", aConsumeEventCallback);
			}

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
				for (EventSubscriber evSub : Registry[str])
				{
					if (evSub.Callback == aConsumeEventCallback)
					{
						Registry[str].erase(std::remove(Registry[str].begin(), Registry[str].end(), evSub), Registry[str].end());
						break;
					}
				}
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
				for (EventSubscriber evSub : consumers)
				{
					if (evSub.Callback >= aStartAddress && evSub.Callback <= aEndAddress)
					{
						consumers.erase(std::remove(consumers.begin(), consumers.end(), evSub), consumers.end());
						refCounter++;
					}
				}
			}
		}

		return refCounter;
	}
}
