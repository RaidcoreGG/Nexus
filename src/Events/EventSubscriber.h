///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EventSubscriber.h
/// Description  :  Contains the EventSubscriber data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EVENTSUBSCRIBER_H
#define EVENTSUBSCRIBER_H

#include "FuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// EventSubscriber data struct
///----------------------------------------------------------------------------------------------------
struct EventSubscriber
{
	signed int Signature;
	EVENT_CONSUME Callback;
};

bool operator==(const EventSubscriber& lhs, const EventSubscriber& rhs);
bool operator!=(const EventSubscriber& lhs, const EventSubscriber& rhs);

#endif
