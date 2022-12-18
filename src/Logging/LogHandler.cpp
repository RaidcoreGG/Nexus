#include <ctime>

#include "LogHandler.h"
#include <thread>

LogHandler* LogHandler::mLogHandler = nullptr;

LogHandler* LogHandler::GetInstance()
{
    if(mLogHandler == nullptr)
    {
        mLogHandler = new LogHandler();
    }
    
    return mLogHandler;
}

void LogHandler::Register(ILogger* aLogger)
{
    mLoggersMutex.lock();

    mLoggers.push_back(aLogger);

    mLoggersMutex.unlock();
}
void LogHandler::WriteEntries()
{
    mLoggersMutex.lock();
    mLogEntriesMutex.lock();

    /* iterate over the subscribed loggers */
    for (size_t i = 0; i < mLoggers.size(); i++)
    {
        LogLevel level = mLoggers[i]->GetLogLevel();

        /* iterate over the logged messages */
        for (size_t j = 0; j < mLogEntries.size(); j++)
        {
            /* send logged message to logger if message log level is lower than logger level */
            if(mLogEntries[j].mLogLevel <= level)
            {
                mLoggers[i]->LogMessage(mLogEntries[j]);
            }
        }
        
    }
    mLogEntries.clear();

    mLogEntriesMutex.unlock();
    mLoggersMutex.unlock();
}

/* Logging functions */
void LogHandler::Log(const char* aMessage)
{
    LogMessage(LogLevel::TRACE, aMessage);
}
void LogHandler::LogCritical(const char* aMessage)
{
    LogMessage(LogLevel::CRITICAL, aMessage);
}
void LogHandler::LogWarning(const char* aMessage)
{
    LogMessage(LogLevel::WARNING, aMessage);
}
void LogHandler::LogInfo(const char* aMessage)
{
    LogMessage(LogLevel::INFO, aMessage);
}
void LogHandler::LogDebug(const char* aMessage)
{
    LogMessage(LogLevel::DEBUG, aMessage);
}

void LogHandler::LogMessage(LogLevel aLogLevel, const char* aMessage)
{
    std::thread([this, aLogLevel, aMessage]()
        {
            mLogEntriesMutex.lock();

            LogEntry entry;
            entry.mLogLevel = aLogLevel;
            entry.mTimestamp = time(NULL);
            entry.mMessage = aMessage;

            mLogEntries.push_back(entry);

            mLogEntriesMutex.unlock();

            WriteEntries();
        }).detach();
}