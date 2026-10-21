///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogMsg.h
/// Description  :  Contains the defintion for a log message entry.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "LogEnum.h"

///----------------------------------------------------------------------------------------------------
/// LogMsg_t Struct
///----------------------------------------------------------------------------------------------------
struct LogMsg_t
{
	ELogLevel   Level          {};
	long long   Time           {};
	int         TimeMsPrecision{};
	std::string Channel        {};
	std::string Message        {};
	int         RepeatCount    {1};
};
