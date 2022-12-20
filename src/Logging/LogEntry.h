#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>

#include "LogLevel.h"

typedef struct LogEntry
{
    LogLevel LogLevel;
    unsigned long long Timestamp;
    std::wstring Message;

    std::wstring ToString();
} LogEntry_t;

#endif