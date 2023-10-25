#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <vector>
#include <mutex>
#include <ctime>
#include <thread>
#include <cstdarg>
#include <algorithm>
#include <chrono>
#include <string>

#include "../Paths.h"

#include "ILogger.h"
#include "LogEntry.h"

#include "ConsoleLogger.h"
#include "FileLogger.h"

namespace LogHandler
{
	extern std::mutex Mutex;
	extern std::vector<ILogger*> Registry;
	extern std::vector<LogEntry> LogEntries;
	extern std::vector<std::string> Channels;

	extern bool IsRunning;
	extern std::thread LoggingThread;
	extern std::vector<LogEntry> QueuedMessages;

	void Initialize();
	void Shutdown();

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
	void ProcessQueueLoop();

	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif