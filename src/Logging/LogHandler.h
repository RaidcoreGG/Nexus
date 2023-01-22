#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <vector>
#include <mutex>

#include "ILogger.h"
#include "LogEntry.h"

typedef void (*LOGGER_LOGA)(ELogLevel aLogLevel, const char* aFmt, ...);
typedef void (*LOGGER_LOGW)(ELogLevel aLogLevel, const wchar_t* aFmt, ...);
typedef void (*LOGGER_ADDREM)(ILogger* aLogger);

namespace LogHandler
{
    extern std::mutex LoggersMutex;
    extern std::vector<ILogger*> Loggers;
    extern std::vector<LogEntry> LogEntries;

    void RegisterLogger(ILogger* aLogger);
    void UnregisterLogger(ILogger* aLogger);

    /* Logging helper functions */
    void Log(const wchar_t* aFmt, ...);
    void LogCritical(const wchar_t* aFmt, ...);
    void LogWarning(const wchar_t* aFmt, ...);
    void LogInfo(const wchar_t* aFmt, ...);
    void LogDebug(const wchar_t* aFmt, ...);

    void Log(const char* aFmt, ...);
    void LogCritical(const char* aFmt, ...);
    void LogWarning(const char* aFmt, ...);
    void LogInfo(const char* aFmt, ...);
    void LogDebug(const char* aFmt, ...);
    
    /* Basic logging functions */
    void LogMessageA(ELogLevel aLogLevel, const char* aFmt, ...);
    void LogMessageW(ELogLevel aLogLevel, const wchar_t* aFmt, ...);

    /* Logging internal functions */
    void LogMessage(ELogLevel aLogLevel, const wchar_t* aFmt, va_list aArgs);
    void LogMessage(ELogLevel aLogLevel, const char* aFmt, va_list aArgs);
}

#endif