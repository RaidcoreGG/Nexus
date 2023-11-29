#include "LogEntry.h"

#include <iomanip>
#include <sstream>

std::string LogEntry::TimestampString(bool aIncludeDate)
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
	std::string str = oss.str();
	return str;
}

std::string LogEntry::ToString(bool aIncludeChannel)
{
	const char* level;

	switch (LogLevel)
	{
		case ELogLevel::CRITICAL:	level = " [CRITICAL] ";     break;
		case ELogLevel::WARNING:	level = " [WARNING] ";      break;
		case ELogLevel::INFO:		level = " [INFO] ";         break;
		case ELogLevel::DEBUG:		level = " [DEBUG] ";        break;

		default:					level = " [TRACE] ";        break;
	}

	std::stringstream oss;
	oss << std::setw(10) << TimestampString();
	if (aIncludeChannel) { oss << std::setw(20) << + "[" + Channel + "]"; }
	oss << std::setw(12) << level;
	std::string str = oss.str() + std::string{Message} + "\n";

	return str;
}