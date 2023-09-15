#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <thread>
#include <algorithm>

#include "../Consts.h"
#include "../Shared.h"

#include "FuncDefs.h"

namespace Events
{
	extern std::mutex											Mutex;
	extern std::map<std::string, std::vector<EVENT_CONSUME>>	Registry;

	/* Raises an event of provided name, passing a pointer to an eventArgs struct. */
	void Raise(const char* aIdentifier, void* aEventData);
	/* Subscribes the provided ConsumeEventCallback function, to the provided event name. */
	void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);
	/* Unsubscribes the provided ConsumeEventCallback function from the provided event name. */
	void Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

	/* Removes all subscribers that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif