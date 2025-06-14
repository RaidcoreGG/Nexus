///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogConst.cpp
/// Description  :  Constant data for logging.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogConst.h"

#include <assert.h>
#include <iomanip>
#include <sstream>

#include "Util/Strings.h"

std::string StringFrom(ELogLevel aLevel)
{
	assert(aLevel != ELogLevel::OFF && aLevel != ELogLevel::ALL);

	switch (aLevel)
	{
		case ELogLevel::CRITICAL: { return "[CRITICAL]"; }
		case ELogLevel::WARNING:  { return "[WARNING]";  }
		case ELogLevel::INFO:     { return "[INFO]";     }
		case ELogLevel::DEBUG:    { return "[DEBUG]";    }
		case ELogLevel::TRACE:    { return "[TRACE]";    }
	}

	return "(null)";
}

std::string TimestampStr(const LogMsg_t* aLogMessage, bool aIncludeDate, bool aMsPrecision)
{
	struct tm timeinfo;
	localtime_s(&timeinfo, (time_t*)&aLogMessage->Time);

	std::stringstream oss;

	if (aIncludeDate)
	{
		oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
	}
	else
	{
		oss << std::put_time(&timeinfo, "%H:%M:%S");
	}

	if (aMsPrecision)
	{
		oss << "." << aLogMessage->TimeMsPrecision;
	}

	return oss.str();
}

std::string ToString(const LogMsg_t* aLogMessage)
{
	/* The null terminator is factored in, so it's effectively 2 spaces padded at the end. */
	static size_t s_TimestampLength  = sizeof("9999-99-99 00:00:00.000 ");
	static size_t s_MaxChannelLength = 12; // Initial value.
	static size_t s_ChannelPadLength = sizeof("[] ");
	static size_t s_LevelLength      = sizeof("[CRITICAL] ");

	/* Push width for all future messages. Does not factor in square brackets and space separator. */
	if (aLogMessage->Channel.length() > s_MaxChannelLength)
	{
		s_MaxChannelLength = aLogMessage->Channel.length();
	}

	std::stringstream oss;
	oss << std::left << std::setw(s_TimestampLength)                       << TimestampStr(aLogMessage, true, true);
	oss << std::left << std::setw(s_MaxChannelLength + s_ChannelPadLength) << + "[" + aLogMessage->Channel + "]  ";
	oss << std::left << std::setw(s_LevelLength)                           << StringFrom(aLogMessage->Level) + "  ";

	std::vector<std::string> parts = String::Split(aLogMessage->Message, "\n");

	for (size_t i = 0; i < parts.size(); i++)
	{
		if (i == 0)
		{
			oss << parts[i] << "\n";
		}
		else
		{
			oss << std::right << std::setw(s_TimestampLength + s_MaxChannelLength + s_ChannelPadLength + s_LevelLength) << " ";
			oss << parts[i] << "\n";
		}
	}

	return oss.str();
}
