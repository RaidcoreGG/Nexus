#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

#include "LogLevel.h"

typedef struct LogEntry
{
    LogLevel mLogLevel;
    unsigned long long mTimestamp;
    const char* mMessage;

    std::string ToString();
} LogEntry_t;

#endif