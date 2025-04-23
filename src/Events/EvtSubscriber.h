///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtSubscriber.h
/// Description  :  Contains the EventSubscriber data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EVTSUBSCRIBER_H
#define EVTSUBSCRIBER_H

#include "EvtFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// EventSubscriber Struct
///----------------------------------------------------------------------------------------------------
struct EventSubscriber
{
	signed int    Signature;
	EVENT_CONSUME Callback;
};

inline bool operator==(const EventSubscriber& lhs, const EventSubscriber& rhs)
{
	return lhs.Signature == rhs.Signature && lhs.Callback == rhs.Callback;
}

inline bool operator!=(const EventSubscriber& lhs, const EventSubscriber& rhs)
{
	return !(lhs == rhs);
}

#endif
