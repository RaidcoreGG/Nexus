#include <ctime>

#include "LogHandler.h"
#include <thread>
#include <cstdarg>
#include <algorithm>
#include <chrono>

namespace LogHandler
{
    std::mutex LoggersMutex;
    std::vector<ILogger*> Loggers;
    std::vector<LogEntry> LogEntries;

    void RegisterLogger(ILogger* aLogger)
    {
        LoggersMutex.lock();

        Loggers.push_back(aLogger);

        LoggersMutex.unlock();
    }
    void UnregisterLogger(ILogger* aLogger)
    {
        LoggersMutex.lock();

        Loggers.erase(std::remove(Loggers.begin(), Loggers.end(), aLogger), Loggers.end());

        LoggersMutex.unlock();
    }

    /* Logging helper functions */
    void Log(const wchar_t* aFmt, ...)          { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::TRACE,    aFmt, args); va_end(args); }
    void LogCritical(const wchar_t* aFmt, ...)  { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::CRITICAL, aFmt, args); va_end(args); }
    void LogWarning(const wchar_t* aFmt, ...)   { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::WARNING,  aFmt, args); va_end(args); }
    void LogInfo(const wchar_t* aFmt, ...)      { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::INFO,     aFmt, args); va_end(args); }
    void LogDebug(const wchar_t* aFmt, ...)     { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::DEBUG,    aFmt, args); va_end(args); }

    void Log(const char* aFmt, ...)             { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::TRACE,    aFmt, args); va_end(args); }
    void LogCritical(const char* aFmt, ...)     { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::CRITICAL, aFmt, args); va_end(args); }
    void LogWarning(const char* aFmt, ...)      { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::WARNING,  aFmt, args); va_end(args); }
    void LogInfo(const char* aFmt, ...)         { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::INFO,     aFmt, args); va_end(args); }
    void LogDebug(const char* aFmt, ...)        { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::DEBUG,    aFmt, args); va_end(args); }

    /* Basic logging functions */
    void LogMessageA(ELogLevel aLogLevel, const char* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(aLogLevel, aFmt, args); va_end(args); }
    void LogMessageW(ELogLevel aLogLevel, const wchar_t* aFmt, ...) { va_list args; va_start(args, aFmt); LogMessage(aLogLevel, aFmt, args); va_end(args); }

    /* Logging internal functions */
    void LogMessage(ELogLevel aLogLevel, const wchar_t* aFmt, va_list aArgs)
    {
        LogEntry entry;
        entry.LogLevel = aLogLevel;
        entry.Timestamp = time(NULL);

        wchar_t buffer[4096];
        vswprintf_s(buffer, 4096, aFmt, aArgs);

        entry.Message = buffer;

        LoggersMutex.lock();
        LogEntries.push_back(entry);

        for (ILogger* logger : Loggers)
        {
            ELogLevel level = logger->GetLogLevel();

            /* send logged message to logger if message log level is lower than logger level */
            if (entry.LogLevel <= level)
            {
                std::thread([logger, entry]()
                {
                    logger->LogMessage(entry);
                }
                ).detach();
            }
        }
        LoggersMutex.unlock();
    }
    void LogMessage(ELogLevel aLogLevel, const char* aFmt, va_list aArgs)
    {
        LogEntry entry;
        entry.LogLevel = aLogLevel;
        entry.Timestamp = time(NULL);

        char buffer[4096];
        vsprintf_s(buffer, 4096, aFmt, aArgs);

        entry.Message = std::wstring(&buffer[0], &buffer[strlen(buffer)]);

        LoggersMutex.lock();
        LogEntries.push_back(entry);

        for (ILogger* logger : Loggers)
        {
            ELogLevel level = logger->GetLogLevel();

            /* send logged message to logger if message log level is lower than logger level */
            if (entry.LogLevel <= level)
            {
                std::thread([logger, entry]()
                {
                    logger->LogMessage(entry);
                }
                ).detach();
            }
        }
        LoggersMutex.unlock();
    }
}