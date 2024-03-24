#ifndef EVENTSUBSCRIBER_H
#define EVENTSUBSCRIBER_H

#include "FuncDefs.h"

struct EventSubscriber
{
	signed int Signature;
	EVENT_CONSUME Callback;
};

bool operator==(const EventSubscriber& lhs, const EventSubscriber& rhs);
bool operator!=(const EventSubscriber& lhs, const EventSubscriber& rhs);

#endif
