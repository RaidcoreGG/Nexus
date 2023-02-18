#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <thread>
#include <algorithm>

#include "../Shared.h"

#include "FuncDefs.h"

namespace Events
{
	extern std::mutex Mutex;
	extern std::map<std::string, std::vector<EVENT_CONSUME>> Registry;

	void Raise(std::string aIdentifier, void* aEventData);										/* Raises an event of provided name, passing a pointer to an eventArgs struct */
	void Subscribe(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback);				/* Subscribes the provided ConsumeEventCallback function, to the provided event name */
	void Unsubscribe(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback);			/* Unsubscribes the provided ConsumeEventCallback function from the provided event name */
}

#endif