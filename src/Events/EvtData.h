///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtData.h
/// Description  :  Contains the EventData struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EVTDATA_H
#define EVTDATA_H

#include <vector>

#include "EvtSubscriber.h"

///----------------------------------------------------------------------------------------------------
/// EventData Struct
///----------------------------------------------------------------------------------------------------
struct EventData
{
	std::vector<EventSubscriber> Subscribers;
	unsigned long long           AmountRaises = 0;
};

#endif
