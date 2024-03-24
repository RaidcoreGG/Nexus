#include "EventHandler.h"

bool operator==(const EventSubscriber& lhs, const EventSubscriber& rhs)
{
	return	lhs.Signature == rhs.Signature && lhs.Callback == rhs.Callback;
}

bool operator!=(const EventSubscriber& lhs, const EventSubscriber& rhs)
{
	return	!(lhs == rhs);
}