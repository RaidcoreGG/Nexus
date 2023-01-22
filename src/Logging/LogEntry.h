#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

#include "ELogLevel.h"

struct LogEntry
{
    ELogLevel LogLevel;
    unsigned long long Timestamp;
    std::wstring Message;

    std::wstring TimestampString(bool aIncludeDate = true);
    std::wstring ToString();
};

#endif