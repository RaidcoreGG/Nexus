///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ILogger.cpp
/// Description  :  Interface for custom loggers.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ILogger.h"

ELogLevel ILogger::GetLogLevel()
{
	return LogLevel;
}

void ILogger::SetLogLevel(ELogLevel aLogLevel)
{
	LogLevel = aLogLevel;
}
