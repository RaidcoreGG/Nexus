///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtData.h
/// Description  :  Contains the EventData_t struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <vector>

#include "EvtSubscriber.h"

///----------------------------------------------------------------------------------------------------
/// EventData_t Struct
///----------------------------------------------------------------------------------------------------
struct EventData_t
{
	std::vector<EventSubscriber_t> Subscribers;
	unsigned long long             AmountRaises = 0;
};
