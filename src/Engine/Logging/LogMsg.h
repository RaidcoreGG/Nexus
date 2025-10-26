///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogMsg.h
/// Description  :  Contains the defintion for a log message entry.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOGMSG_H
#define LOGMSG_H

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

#endif
