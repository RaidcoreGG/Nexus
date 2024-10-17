///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CFileLogger.cpp
/// Description  :  Custom logger to print to a log file.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CFileLogger.h"

CFileLogger::CFileLogger(ELogLevel aLogLevel, std::filesystem::path aPath)
{
	LogLevel = aLogLevel;
	File.open(aPath, std::ios_base::out, SH_DENYWR);
}

CFileLogger::~CFileLogger()
{
	File.flush();
	File.close();
}

void CFileLogger::LogMessage(LogEntry* aLogEntry)
{
	const std::lock_guard<std::mutex> lock(Mutex);

	if (aLogEntry->RepeatCount == 1)
	{
		File << aLogEntry->ToString(true, true, true);
		File.flush();
	}
}
