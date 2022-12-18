#include <iomanip>
#include <sstream>

#include "LogEntry.h"

std::string LogEntry::ToString()
{
    const char* level;

    switch (mLogLevel)
    {
        case LogLevel::CRITICAL:    level = " [CRITICAL] ";     break;
        case LogLevel::WARNING:     level = " [WARNING] ";      break;
        case LogLevel::INFO:        level = " [INFO] ";         break;
        case LogLevel::DEBUG:       level = " [DEBUG] ";        break;

        default:                    level = " [TRACE] ";        break;
    }

    struct tm timeinfo;
    localtime_s(&timeinfo, (time_t*)&mTimestamp);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    oss << std::setw(12) << level;
    std::string str = oss.str() + mMessage + "\n";

    return str;
}