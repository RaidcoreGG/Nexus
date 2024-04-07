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
	void ADDONAPI_RaiseEvent(const char* aIdentifier, void* aEventData)
	{
		Raise(aIdentifier, aEventData);
	}

	void ADDONAPI_RaiseNotification(const char* aIdentifier)
	{
		Raise(aIdentifier, nullptr);
	}

	void ADDONAPI_RaiseEventTargeted(signed int aSignature, const char* aIdentifier, void* aEventData)
	{
		Raise(aSignature, aIdentifier, aEventData);
	}

	void ADDONAPI_RaiseNotificationTargeted(signed int aSignature, const char* aIdentifier)
	{
		Raise(aSignature, aIdentifier, nullptr);
	}
}

namespace Events
{
	std::mutex							Mutex;
	std::map<std::string, EventData>	Registry;

	void Raise(const char* aIdentifier, void* aEventData)
	{
		Registry[aIdentifier].AmountRaises++;

		//auto start_time = std::chrono::high_resolution_clock::now();

		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (EventSubscriber sub : Registry[str].Subscribers)
			{
				sub.Callback(aEventData);
			}
		}

		//auto end_time = std::chrono::high_resolution_clock::now();
		//auto time = end_time - start_time;
		//LogDebug(CH_EVENTS, u8"Executed event (%s) in %uµs.", aIdentifier, time / std::chrono::microseconds(1));
	}

	void Raise(signed int aSignature, const char* aIdentifier, void* aEventData)
	{
		Registry[aIdentifier].AmountRaises++;

		std::string str = aIdentifier;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (EventSubscriber sub : Registry[str].Subscribers)
			{
				if (sub.Signature == aSignature)
				{
					sub.Callback(aEventData);
					break;
				}
			}
		}
	}

	void Raise(const char* aIdentifier)
	{
		Raise(aIdentifier, nullptr);
	}

	void Raise(signed int aSignature, const char* aIdentifier)
	{
		Raise(aSignature, aIdentifier, nullptr);
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

			Registry[str].Subscribers.push_back(sub);

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
				for (EventSubscriber evSub : Registry[str].Subscribers)
				{
					if (evSub.Callback == aConsumeEventCallback)
					{
						Registry[str].Subscribers.erase(std::remove(Registry[str].Subscribers.begin(), Registry[str].Subscribers.end(), evSub), Registry[str].Subscribers.end());
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
			for (auto& [identifier, ev] : Registry)
			{
				for (EventSubscriber evSub : ev.Subscribers)
				{
					if (evSub.Callback >= aStartAddress && evSub.Callback <= aEndAddress)
					{
						ev.Subscribers.erase(std::remove(ev.Subscribers.begin(), ev.Subscribers.end(), evSub), ev.Subscribers.end());
						refCounter++;
					}
				}
			}
		}

		return refCounter;
	}
}
