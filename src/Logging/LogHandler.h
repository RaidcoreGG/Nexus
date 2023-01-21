#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <vector>
#include <mutex>

#include "ILogger.h"
#include "LogEntry.h"

typedef void (*LOGGER_LOGA)(const char* aFmt, ...);
typedef void (*LOGGER_LOGW)(const wchar_t* aFmt, ...);
typedef void (*LOGGER_ADDREM)(ILogger* aLogger);

class LogHandler
{
    public:
        LogHandler(const LogHandler&) = delete;             // copy constructor
        LogHandler& operator=(const LogHandler&) = delete;  // assign operator
        static LogHandler* GetInstance();

        void Register(ILogger* aLogger);
        void Unregister(ILogger* aLogger);

        /* Logging functions */
        void Log(           const wchar_t* aFmt, ...);
        void LogCritical(   const wchar_t* aFmt, ...);
        void LogWarning(    const wchar_t* aFmt, ...);
        void LogInfo(       const wchar_t* aFmt, ...);
        void LogDebug(      const wchar_t* aFmt, ...);

        void Log(           const char* aFmt, ...);
        void LogCritical(   const char* aFmt, ...);
        void LogWarning(    const char* aFmt, ...);
        void LogInfo(       const char* aFmt, ...);
        void LogDebug(      const char* aFmt, ...);

        std::vector<LogEntry> LogEntries;

    private:
        LogHandler() = default;

        void LogMessage(ELogLevel aLogLevel, const wchar_t* aFmt, va_list aArgs);
        void LogMessage(ELogLevel aLogLevel, const char* aFmt, va_list aArgs);

        static LogHandler* Instance;
        std::vector<ILogger*> Loggers;
        std::mutex LoggersMutex;
};

#endif