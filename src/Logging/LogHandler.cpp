#include <ctime>

#include "LogHandler.h"
#include <thread>
#include <cstdarg>
#include <algorithm>
#include <chrono>

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
void LogHandler::Log(           const wchar_t* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::TRACE,    aFmt, args); va_end(args); }
void LogHandler::LogCritical(   const wchar_t* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::CRITICAL, aFmt, args); va_end(args); }
void LogHandler::LogWarning(    const wchar_t* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::WARNING,  aFmt, args); va_end(args); }
void LogHandler::LogInfo(       const wchar_t* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::INFO,     aFmt, args); va_end(args); }
void LogHandler::LogDebug(      const wchar_t* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::DEBUG,    aFmt, args); va_end(args); }

void LogHandler::Log(           const char* aFmt, ...)       { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::TRACE,    aFmt, args); va_end(args); }
void LogHandler::LogCritical(   const char* aFmt, ...)       { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::CRITICAL, aFmt, args); va_end(args); }
void LogHandler::LogWarning(    const char* aFmt, ...)       { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::WARNING,  aFmt, args); va_end(args); }
void LogHandler::LogInfo(       const char* aFmt, ...)       { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::INFO,     aFmt, args); va_end(args); }
void LogHandler::LogDebug(      const char* aFmt, ...)       { va_list args; va_start(args, aFmt); LogMessage(ELogLevel::DEBUG,    aFmt, args); va_end(args); }

void LogHandler::LogMessage(ELogLevel aLogLevel, const wchar_t* aFmt, va_list aArgs)
{
    LogEntry entry;
    entry.LogLevel = aLogLevel;
    entry.Timestamp = time(NULL);

    wchar_t buffer[400];
    vswprintf_s(buffer, 400, aFmt, aArgs);

    entry.Message = buffer;

    for (ILogger* logger : Loggers)
    {
        ELogLevel level = logger->GetLogLevel();

        /* send logged message to logger if message log level is lower than logger level */
        if (entry.LogLevel <= level)
        {
            std::thread([logger, entry, this]() { LoggersMutex.lock(); logger->LogMessage(entry); LoggersMutex.unlock(); }).detach();
        }
    }
}

void LogHandler::LogMessage(ELogLevel aLogLevel, const char* aFmt, va_list aArgs)
{
    size_t sz = strlen(aFmt) + 1;
    wchar_t* wc = new wchar_t[sz];
    mbstowcs_s(nullptr, wc, sz, aFmt, sz);
    LogMessage(aLogLevel, wc, aArgs);
}