#include "ILogger.h"

LogLevel ILogger::GetLogLevel()
{
    return mLogLevel;
}
void ILogger::SetLogLevel(LogLevel aLogLevel)
{
    mLogLevel = aLogLevel;
}