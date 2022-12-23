#include "ILogger.h"

ELogLevel ILogger::GetLogLevel()
{
    return LogLevel;
}

void ILogger::SetLogLevel(ELogLevel aLogLevel)
{
    LogLevel = aLogLevel;
}