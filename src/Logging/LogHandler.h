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

        void Register(ILogger*);
        void Unregister(ILogger*);

        /* Logging functions */
        void Log(           const wchar_t*, ...);
        void LogCritical(   const wchar_t*, ...);
        void LogWarning(    const wchar_t*, ...);
        void LogInfo(       const wchar_t*, ...);
        void LogDebug(      const wchar_t*, ...);

        void Log(           const char*, ...);
        void LogCritical(   const char*, ...);
        void LogWarning(    const char*, ...);
        void LogInfo(       const char*, ...);
        void LogDebug(      const char*, ...);

    private:
        LogHandler() = default;

        /* internal logging function */
        void LogMessage(LogLevel, const wchar_t*, va_list);
        void LogMessage(LogLevel, const char*, va_list);

        static LogHandler* Instance;

        std::vector<ILogger*> Loggers;
        std::vector<LogEntry> LogEntries;

        std::mutex LoggersMutex;
};

#endif