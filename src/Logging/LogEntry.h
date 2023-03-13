#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>
#include <iomanip>
#include <sstream>

#include "ELogLevel.h"

struct LogEntry
{
	ELogLevel LogLevel;
	unsigned long long Timestamp;
	std::string Channel;
	std::string Message;

	std::string TimestampString(bool aIncludeDate = true);
	std::string ToString(bool aIncludeChannel = true);
};

#endif