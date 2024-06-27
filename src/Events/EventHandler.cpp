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

#include "Consts.h"
#include "Shared.h"

#include "Loader/Loader.h"
#include "Loader/ArcDPS.h"

namespace Events
{

	void ADDONAPI_Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		EventApi->Subscribe(aIdentifier, aConsumeEventCallback);
	}

	void ADDONAPI_Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
	{
		EventApi->Unsubscribe(aIdentifier, aConsumeEventCallback);
	}

	void ADDONAPI_RaiseEvent(const char* aIdentifier, void* aEventData)
	{
		EventApi->Raise(aIdentifier, aEventData);
	}

	void ADDONAPI_RaiseNotification(const char* aIdentifier)
	{
		EventApi->Raise(aIdentifier, nullptr);
	}

	void ADDONAPI_RaiseEventTargeted(signed int aSignature, const char* aIdentifier, void* aEventData)
	{
		EventApi->Raise(aSignature, aIdentifier, aEventData);
	}

	void ADDONAPI_RaiseNotificationTargeted(signed int aSignature, const char* aIdentifier)
	{
		EventApi->Raise(aSignature, aIdentifier, nullptr);
	}
}

void CEventApi::Raise(const char* aIdentifier, void* aEventData)
{
	this->Registry[aIdentifier].AmountRaises++;

	//auto start_time = std::chrono::high_resolution_clock::now();

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	
	for (EventSubscriber sub : this->Registry[str].Subscribers)
	{
		sub.Callback(aEventData);
	}

	//auto end_time = std::chrono::high_resolution_clock::now();
	//auto time = end_time - start_time;
	//Logger->Debug(CH_EVENTS, u8"Executed event (%s) in %uµs.", aIdentifier, time / std::chrono::microseconds(1));
}

void CEventApi::Raise(signed int aSignature, const char* aIdentifier, void* aEventData)
{
	this->Registry[aIdentifier].AmountRaises++;

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	
	for (EventSubscriber sub : this->Registry[str].Subscribers)
	{
		if (sub.Signature == aSignature)
		{
			sub.Callback(aEventData);
			break;
		}
	}
}

void CEventApi::Raise(const char* aIdentifier)
{
	this->Raise(aIdentifier, nullptr);
}

void CEventApi::Raise(signed int aSignature, const char* aIdentifier)
{
	this->Raise(aSignature, aIdentifier, nullptr);
}

void CEventApi::Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
{
	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	
	EventSubscriber sub{};
	sub.Callback = aConsumeEventCallback;

	for (auto addon : Loader::Addons)
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

	this->Registry[str].Subscribers.push_back(sub);

	if (sub.Signature == 0)
	{
		Logger->Warning(CH_EVENTS, "Event registered but no addon address space matches function pointer. %p", aConsumeEventCallback);
	}

	/* dirty hack for arcdps (I hate my life) */
	if ((str == "EV_ARCDPS_COMBATEVENT_LOCAL_RAW" || str == "EV_ARCDPS_COMBATEVENT_SQUAD_RAW") && !ArcDPS::IsLoaded)
	{
		ArcDPS::Detect();
	}
}

void CEventApi::Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
{
	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	
	if (this->Registry.find(str) == this->Registry.end())
	{
		return;
	}

	for (EventSubscriber evSub : this->Registry[str].Subscribers)
	{
		if (evSub.Callback == aConsumeEventCallback)
		{
			this->Registry[str].Subscribers.erase(std::remove(this->Registry[str].Subscribers.begin(), this->Registry[str].Subscribers.end(), evSub), this->Registry[str].Subscribers.end());
			break;
		}
	}
}

int CEventApi::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	
	for (auto& [identifier, ev] : this->Registry)
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

	return refCounter;
}

std::unordered_map<std::string, EventData> CEventApi::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}
