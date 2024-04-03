///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EventSubscriber.cpp
/// Description  :  Contains the EventSubscriber data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "EventSubscriber.h"

bool operator==(const EventSubscriber& lhs, const EventSubscriber& rhs)
{
	return	lhs.Signature == rhs.Signature && lhs.Callback == rhs.Callback;
}

bool operator!=(const EventSubscriber& lhs, const EventSubscriber& rhs)
{
	return	!(lhs == rhs);
}
