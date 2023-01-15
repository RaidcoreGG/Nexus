#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

#include "ELogLevel.h"

typedef struct LogEntry
{
    ELogLevel LogLevel;
    unsigned long long Timestamp;
    std::wstring Message;

    std::wstring TimestampString(bool aIncludeDate = true);
    std::wstring ToString();
} LogEntry_t;

#endif