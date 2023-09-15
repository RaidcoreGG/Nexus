#ifndef EVENTS_FUNCDEFS_H
#define EVENTS_FUNCDEFS_H

#include <string>

typedef void (*EVENT_CONSUME)(void* aEventArgs);
typedef void (*EVENTS_RAISE)(const char* aIdentifier, void* aEventData);
typedef void (*EVENTS_SUBSCRIBE)(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

#endif