#ifndef ILOGGER_H
#define ILOGGER_H

#include "LogLevel.h"
#include "LogEntry.h"

#include <mutex>

class ILogger
{
    public:
        ILogger() = default;
        virtual ~ILogger() = default;

        LogLevel GetLogLevel();
        void SetLogLevel(LogLevel);

        virtual void LogMessage(LogEntry) = 0;

    protected:
        LogLevel Level;
        std::mutex MessageMutex;
};

#endif