///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogEntry.h
/// Description  :  Contains the LogEntry data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

#include "ELogLevel.h"

///----------------------------------------------------------------------------------------------------
/// LogEntry data struct
///----------------------------------------------------------------------------------------------------
struct LogEntry
{
	ELogLevel LogLevel;
	unsigned long long Timestamp;
	int TimestampMilliseconds;
	std::string Channel;
	std::string Message;
	int RepeatCount = 1;

	///----------------------------------------------------------------------------------------------------
	/// TimestampString:
	/// 	Converts the timestamp of the log message to a string.
	///----------------------------------------------------------------------------------------------------
	std::string TimestampString(bool aIncludeDate = true, bool aIncludeMs = false);

	///----------------------------------------------------------------------------------------------------
	/// ToString:
	/// 	Converts the log message to a printable string.
	///----------------------------------------------------------------------------------------------------
	std::string ToString(bool aIncludeChannel = true, bool aIncludeDate = true, bool aIncludeMs = false);
};

#endif
