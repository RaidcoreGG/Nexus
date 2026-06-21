///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EventApi.cpp
/// Description  :  Provides functions raise and receive events.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "EvtApi.h"

#include "EvtSubscriber.h"

CEventApi::CEventApi(CLoader* aLoader) : IRefCleaner("EventApi")
{
	this->Loader = aLoader;
}

CEventApi::~CEventApi()
{
}

void CEventApi::Raise(const char* aIdentifier, void* aEventData)
{
	if (aIdentifier == nullptr) { return; }

	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);
	
	if (it == this->Registry.end())
	{
		return;
	}

	it->second.AmountRaises++;

	for (EventSubscriber_t& sub : it->second.Subscribers)
	{
		sub.Callback(aEventData);
	}
}

void CEventApi::Raise(uint32_t aSignature, const char* aIdentifier, void* aEventData)
{
	if (aIdentifier == nullptr) { return; }

	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it == this->Registry.end())
	{
		return;
	}

	it->second.AmountRaises++;

	for (EventSubscriber_t& sub : it->second.Subscribers)
	{
		if (sub.Signature == aSignature)
		{
			sub.Callback(aEventData);
		}
	}
}

void CEventApi::Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
{
	if (aIdentifier == nullptr)           { return; }
	if (aConsumeEventCallback == nullptr) { return; }

	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);
	
	EventSubscriber_t sub{};
	sub.Callback = aConsumeEventCallback;

	IAddon* owner = this->Loader->GetOwner(aConsumeEventCallback);

	sub.Signature = owner != nullptr ? owner->GetSignature() : 0;

	auto it = this->Registry.find(aIdentifier);

	/* Emplace new event or add subscriber to existing. */
	if (it == this->Registry.end())
	{
		EventData_t ev{};
		ev.Subscribers.push_back(sub);

		this->Registry.emplace(aIdentifier, ev);
	}
	else
	{
		it->second.Subscribers.push_back(sub);
	}
}

void CEventApi::Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
{
	if (aIdentifier == nullptr) { return; }

	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);
	
	auto it = this->Registry.find(aIdentifier);

	if (it == this->Registry.end())
	{
		return;
	}

	it->second.Subscribers.erase(
		std::remove_if(
			it->second.Subscribers.begin(),
			it->second.Subscribers.end(),
			[aConsumeEventCallback](EventSubscriber_t& sub)
			{
				return aConsumeEventCallback == sub.Callback;
			}
		),
		it->second.Subscribers.end()
	);
}

uint32_t CEventApi::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	uint32_t refCounter = 0;

	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);
	
	for (auto& [identifier, ev] : this->Registry)
	{
		ev.Subscribers.erase(
			std::remove_if(
				ev.Subscribers.begin(),
				ev.Subscribers.end(),
				[&refCounter, aStartAddress, aEndAddress](EventSubscriber_t& sub)
				{
					if (sub.Callback >= aStartAddress && sub.Callback <= aEndAddress)
					{
						refCounter++;
						return true;
					}
					return false;
				}
			),
			ev.Subscribers.end()
		);
	}

	return refCounter;
}

std::unordered_map<std::string, EventData_t> CEventApi::GetRegistry() const
{
	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

	return this->Registry;
}
