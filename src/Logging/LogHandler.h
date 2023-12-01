#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <mutex>
#include <vector>
#include <string>

#include "ILogger.h"
#include "LogEntry.h"
#include "ELogLevel.h"

namespace LogHandler
{
	extern std::mutex Mutex;
	extern std::vector<ILogger*> Registry;
	extern std::vector<LogEntry> LogEntries;
	extern std::vector<std::string> Channels;

	void Initialize();

	void RegisterLogger(ILogger* aLogger);
	void UnregisterLogger(ILogger* aLogger);

	/* Logging helper functions */
	void Log(std::string aChannel, const char* aFmt, ...);
	void LogCritical(std::string aChannel, const char* aFmt, ...);
	void LogWarning(std::string aChannel, const char* aFmt, ...);
	void LogInfo(std::string aChannel, const char* aFmt, ...);
	void LogDebug(std::string aChannel, const char* aFmt, ...);
	
	/* Basic logging functions */
	void LogMessageA(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...);
	void LogMessageAddon(ELogLevel aLogLevel, const char* aStr);

	/* Logging internal functions */
	void LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs);
}

#endif