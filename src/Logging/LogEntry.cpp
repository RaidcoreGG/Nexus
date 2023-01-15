#include <iomanip>
#include <sstream>

#include "LogEntry.h"

std::wstring LogEntry::TimestampString(bool aIncludeDate)
{
    struct tm timeinfo;
    localtime_s(&timeinfo, (time_t*)&Timestamp);

    std::wstringstream oss;
    if (aIncludeDate)
    {
        oss << std::put_time(&timeinfo, L"%Y-%m-%d %H:%M:%S");
    }
    else
    {
        oss << std::put_time(&timeinfo, L"%H:%M:%S");
    }
    std::wstring str = oss.str();
    return str;
}

std::wstring LogEntry::ToString()
{
    const wchar_t* level;

    switch (LogLevel)
    {
        case ELogLevel::CRITICAL:    level = L" [CRITICAL] ";     break;
        case ELogLevel::WARNING:     level = L" [WARNING] ";      break;
        case ELogLevel::INFO:        level = L" [INFO] ";         break;
        case ELogLevel::DEBUG:       level = L" [DEBUG] ";        break;

        default:                    level = L" [TRACE] ";        break;
    }

    std::wstringstream oss;
    oss << TimestampString();
    oss << std::setw(12) << level;
    std::wstring str = oss.str() + std::wstring{Message} + L"\n";

    return str;
}