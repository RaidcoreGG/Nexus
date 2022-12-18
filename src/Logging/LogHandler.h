#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <vector>
#include <mutex>

#include "ILogger.h"
#include "LogEntry.h"

class LogHandler
{
    public:
        LogHandler(const LogHandler&) = delete;             // copy constructor
        LogHandler& operator=(const LogHandler&) = delete;  // assign operator
        static LogHandler* GetInstance();

        void Register(ILogger* aLogger);
        void WriteEntries(); /* Synonymous with a flush */

        /* Logging functions */
        void Log(           const char* aMessage   );
        void LogCritical(   const char* aMessage   );
        void LogWarning(    const char* aMessage   );
        void LogInfo(       const char* aMessage   );
        void LogDebug(      const char* aMessage   );

    private:
        LogHandler() = default;

        /* internal logging function */
        void LogMessage(LogLevel aLogLevel, const char* aMessage);

        static LogHandler* mLogHandler;

        std::vector<ILogger*> mLoggers;
        std::vector<LogEntry> mLogEntries;

        std::mutex mLoggersMutex;
        std::mutex mLogEntriesMutex;
};

#endif