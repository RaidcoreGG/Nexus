#ifndef EVENTS_FUNCDEFS_H
#define EVENTS_FUNCDEFS_H

#include <string>

typedef void (*EVENT_CONSUME)(void* aEventArgs);
typedef void (*EVENTS_RAISE)(std::string aIdentifier, void* aEventData);
typedef void (*EVENTS_SUBSCRIBE)(std::string aIdentifier, EVENT_CONSUME aConsumeEventCallback);

#endif