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
	void DeregisterLogger(ILogger* aLogger);

	/* Logging helper functions */
	void Log(const std::string& aChannel, const char* aFmt, ...);
	void LogCritical(const std::string& aChannel, const char* aFmt, ...);
	void LogWarning(const std::string& aChannel, const char* aFmt, ...);
	void LogInfo(const std::string& aChannel, const char* aFmt, ...);
	void LogDebug(const std::string& aChannel, const char* aFmt, ...);
	
	/* Basic logging functions */
	void LogMessageV(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...);
	void ADDONAPI_LogMessage(ELogLevel aLogLevel, const char* aStr);
	void ADDONAPI_LogMessage2(ELogLevel aLogLevel, const char* aChannel, const char* aStr);

	/* Logging internal functions */
	void LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs);
	void LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aMsg);
}

#endif