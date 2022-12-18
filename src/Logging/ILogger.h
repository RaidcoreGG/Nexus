#ifndef ILOGGER_H
#define ILOGGER_H

#include "LogLevel.h"
#include "LogEntry.h"

class ILogger
{
    public:
        ILogger() = default;
        virtual ~ILogger() = default;

        LogLevel GetLogLevel();
        void SetLogLevel(LogLevel aLogLevel);

        virtual void LogMessage(LogEntry aLogEntry) = 0;

    protected:
        LogLevel mLogLevel;
};

#endif