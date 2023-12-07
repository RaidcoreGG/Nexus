#include "FileLogger.h"

FileLogger::FileLogger(ELogLevel aLogLevel, std::filesystem::path aPath)
{
	LogLevel = aLogLevel;
	File.open(aPath, std::ios_base::out, SH_DENYWR);
}

FileLogger::~FileLogger()
{
	File.flush();
	File.close();
}

void FileLogger::LogMessage(LogEntry aLogEntry)
{
	MessageMutex.lock();
	{
		File << aLogEntry.ToString();
		File.flush();
	}
	MessageMutex.unlock();
}