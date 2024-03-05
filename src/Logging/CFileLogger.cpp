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

void CFileLogger::LogMessage(LogEntry aLogEntry)
{
	MessageMutex.lock();
	{
		File << aLogEntry.ToString();
		File.flush();
	}
	MessageMutex.unlock();
}