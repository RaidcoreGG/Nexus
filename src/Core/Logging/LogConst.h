///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogConst.h
/// Description  :  Constant data for logging.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "LogEnum.h"
#include "LogMsg.h"

///----------------------------------------------------------------------------------------------------
/// StringFrom:
/// 	Converts the log level to a string.
///----------------------------------------------------------------------------------------------------
std::string StringFrom(ELogLevel aLevel);

///----------------------------------------------------------------------------------------------------
/// TimestampStr:
/// 	Returns a timestamp string from a log message.
/// 	aIncludeDate: Controls whether the date should be appended.
/// 	aMsPrecision: Controls whether milliseconds should be appended.
///----------------------------------------------------------------------------------------------------
std::string TimestampStr(const LogMsg_t* aLogMessage, bool aIncludeDate, bool aMsPrecision);

///----------------------------------------------------------------------------------------------------
/// ToString:
/// 	Converts the log message to a fixed-width printable string.
///----------------------------------------------------------------------------------------------------
std::string ToString(const LogMsg_t* aLogMessage);
