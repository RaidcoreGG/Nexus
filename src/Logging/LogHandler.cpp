#include <ctime>

#include "LogHandler.h"
#include <thread>
#include <cstdarg>
#include <algorithm>

LogHandler* LogHandler::Instance = nullptr;

LogHandler* LogHandler::GetInstance()
{
    if(Instance == nullptr)
    {
        Instance = new LogHandler();
    }
    
    return Instance;
}

void LogHandler::Register(ILogger* aLogger)
{
    LoggersMutex.lock();

    Loggers.push_back(aLogger);

    LoggersMutex.unlock();
}

void LogHandler::Unregister(ILogger* aLogger)
{
    LoggersMutex.lock();

    Loggers.erase(std::remove(Loggers.begin(), Loggers.end(), aLogger), Loggers.end());

    LoggersMutex.unlock();
}

/* Logging functions */
void LogHandler::Log(           const wchar_t* fmt, ...)    { va_list args; va_start(args, fmt); LogMessage(ELogLevel::TRACE,    fmt, args); va_end(args); }
void LogHandler::LogCritical(   const wchar_t* fmt, ...)    { va_list args; va_start(args, fmt); LogMessage(ELogLevel::CRITICAL, fmt, args); va_end(args); }
void LogHandler::LogWarning(    const wchar_t* fmt, ...)    { va_list args; va_start(args, fmt); LogMessage(ELogLevel::WARNING,  fmt, args); va_end(args); }
void LogHandler::LogInfo(       const wchar_t* fmt, ...)    { va_list args; va_start(args, fmt); LogMessage(ELogLevel::INFO,     fmt, args); va_end(args); }
void LogHandler::LogDebug(      const wchar_t* fmt, ...)    { va_list args; va_start(args, fmt); LogMessage(ELogLevel::DEBUG,    fmt, args); va_end(args); }

void LogHandler::Log(           const char* fmt, ...)       { va_list args; va_start(args, fmt); LogMessage(ELogLevel::TRACE,    fmt, args); va_end(args); }
void LogHandler::LogCritical(   const char* fmt, ...)       { va_list args; va_start(args, fmt); LogMessage(ELogLevel::CRITICAL, fmt, args); va_end(args); }
void LogHandler::LogWarning(    const char* fmt, ...)       { va_list args; va_start(args, fmt); LogMessage(ELogLevel::WARNING,  fmt, args); va_end(args); }
void LogHandler::LogInfo(       const char* fmt, ...)       { va_list args; va_start(args, fmt); LogMessage(ELogLevel::INFO,     fmt, args); va_end(args); }
void LogHandler::LogDebug(      const char* fmt, ...)       { va_list args; va_start(args, fmt); LogMessage(ELogLevel::DEBUG,    fmt, args); va_end(args); }

void LogHandler::LogMessage(ELogLevel aLogLevel, const wchar_t* fmt, va_list args)
{
    LogEntry entry;
    entry.LogLevel = aLogLevel;
    entry.Timestamp = time(NULL);

    wchar_t buffer[400];
    vswprintf_s(buffer, 400, fmt, args);

    entry.Message = buffer;

    for (ILogger* logger : Loggers)
    {
        ELogLevel level = logger->GetLogLevel();

        /* send logged message to logger if message log level is lower than logger level */
        if (entry.LogLevel <= level)
        {
            std::thread([logger, entry]() { logger->LogMessage(entry); }).detach();
        }
    }
}

void LogHandler::LogMessage(ELogLevel aLogLevel, const char* aMessage, va_list args)
{
    size_t sz = strlen(aMessage) + 1;
    wchar_t* wc = new wchar_t[sz];
    mbstowcs_s(nullptr, wc, sz, aMessage, sz);
    LogMessage(aLogLevel, wc, args);
}