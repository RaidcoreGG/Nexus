#ifndef ILOGGER_H
#define ILOGGER_H

#include "ELogLevel.h"
#include "LogEntry.h"

#include <mutex>

class ILogger
{
    public:
        ILogger() = default;
        virtual ~ILogger() = default;

        ELogLevel GetLogLevel();
        void SetLogLevel(ELogLevel);

        virtual void LogMessage(LogEntry) = 0;

    protected:
        ELogLevel LogLevel;
        std::mutex MessageMutex;
};

#endif