///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogEntry.cpp
/// Description  :  Contains the LogEntry data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogEntry.h"

#include <iomanip>
#include <sstream>

#include "Util/Strings.h"

std::string LogEntry::TimestampString(bool aIncludeDate, bool aIncludeMs)
{
	struct tm timeinfo;
	localtime_s(&timeinfo, (time_t*)&Timestamp);

	std::stringstream oss;

	if (aIncludeDate)
	{
		oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
	}
	else
	{
		oss << std::put_time(&timeinfo, "%H:%M:%S");
	}

	if (aIncludeMs)
	{
		oss << "." << TimestampMilliseconds;
	}

	std::string str = oss.str();
	return str;
}

std::string LogEntry::ToString(bool aIncludeChannel, bool aIncludeDate, bool aIncludeMs)
{
	const char* level;

	switch (LogLevel)
	{
		case ELogLevel::CRITICAL: level = " [CRITICAL] ";     break;
		case ELogLevel::WARNING:  level = " [WARNING] ";      break;
		case ELogLevel::INFO:     level = " [INFO] ";         break;
		case ELogLevel::DEBUG:    level = " [DEBUG] ";        break;

		default:                  level = " [TRACE] ";        break;
	}

	std::stringstream oss;
	oss << std::left << std::setw(26) << TimestampString(aIncludeDate, aIncludeMs);
	if (aIncludeChannel)
	{
		oss << std::right << std::setw(24) << + "[" + Channel + "]";
	}
	oss << std::right << std::setw(12) << level;

	/* remove trailing newline from message */
	if (String::EndsWith(Message, "\n"))
	{
		Message = Message.substr(0, Message.length() - 1);
	}

	std::vector<std::string> parts = String::Split(Message, "\n");

	for (size_t i = 0; i < parts.size(); i++)
	{
		if (i == 0)
		{
			oss << parts[i] << "\n";
		}
		else
		{
			oss << std::right << std::setw(62) << " "; // 62 magic number is total length of padded prefix (timestamp + channel + level)
			oss << parts[i] << "\n";
		}
	}

	return oss.str();
}
