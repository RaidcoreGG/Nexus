#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <vector>
#include <mutex>
#include <ctime>
#include <thread>
#include <cstdarg>
#include <algorithm>
#include <chrono>

#include "ILogger.h"
#include "LogEntry.h"

namespace LogHandler
{
    extern std::mutex LoggersMutex;
    extern std::vector<ILogger*> Loggers;
    extern std::vector<LogEntry> LogEntries;

    void RegisterLogger(ILogger* aLogger);
    void UnregisterLogger(ILogger* aLogger);

    /* Logging helper functions */
    void Log(const char* aFmt, ...);
    void LogCritical(const char* aFmt, ...);
    void LogWarning(const char* aFmt, ...);
    void LogInfo(const char* aFmt, ...);
    void LogDebug(const char* aFmt, ...);
    
    /* Basic logging functions */
    void LogMessageA(ELogLevel aLogLevel, const char* aFmt, ...);

    /* Logging internal functions */
    void LogMessage(ELogLevel aLogLevel, const char* aFmt, va_list aArgs);
}

#endif