///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtSubscriber.h
/// Description  :  Contains the EventSubscriber_t data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "EvtFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// EventSubscriber_t Struct
///----------------------------------------------------------------------------------------------------
struct EventSubscriber_t
{
	uint32_t      Signature;
	EVENT_CONSUME Callback;
};

inline bool operator==(const EventSubscriber_t& lhs, const EventSubscriber_t& rhs)
{
	return lhs.Signature == rhs.Signature && lhs.Callback == rhs.Callback;
}

inline bool operator!=(const EventSubscriber_t& lhs, const EventSubscriber_t& rhs)
{
	return !(lhs == rhs);
}
