#include "ILogger.h"

LogLevel ILogger::GetLogLevel()
{
    return Level;
}

void ILogger::SetLogLevel(LogLevel aLogLevel)
{
    Level = aLogLevel;
}