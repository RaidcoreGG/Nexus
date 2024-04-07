///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogHandler.h
/// Description  :  Provides logging functions and allows for custom logging implementations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <mutex>
#include <vector>
#include <string>

#include "ILogger.h"
#include "LogEntry.h"
#include "ELogLevel.h"

///----------------------------------------------------------------------------------------------------
/// LogHandler Namespace
///----------------------------------------------------------------------------------------------------
namespace LogHandler
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_LogMessage:
	/// 	[Revision 1] Logs a message.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_LogMessage(ELogLevel aLogLevel, const char* aStr);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_LogMessage2:
	/// 	[Revision 2] Logs a message with a custom channel.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_LogMessage2(ELogLevel aLogLevel, const char* aChannel, const char* aStr);
}

///----------------------------------------------------------------------------------------------------
/// LogHandler Namespace
///----------------------------------------------------------------------------------------------------
namespace LogHandler
{
	extern std::mutex					Mutex;
	extern std::vector<ILogger*>		Registry;
	extern std::vector<LogEntry>		LogEntries;
	extern std::vector<std::string>		Channels;

	///----------------------------------------------------------------------------------------------------
	/// RegisterLogger:
	/// 	Registers a logger.
	///----------------------------------------------------------------------------------------------------
	void RegisterLogger(ILogger* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// DeregisterLogger:
	/// 	Deregisters a logger.
	///----------------------------------------------------------------------------------------------------
	void DeregisterLogger(ILogger* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// LogCritical:
	/// 	Logs a message with level Critical.
	///----------------------------------------------------------------------------------------------------
	void LogCritical(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// LogWarning:
	/// 	Logs a message with level Warning.
	///----------------------------------------------------------------------------------------------------
	void LogWarning(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// LogInfo:
	/// 	Logs a message with level Info.
	///----------------------------------------------------------------------------------------------------
	void LogInfo(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// LogDebug:
	/// 	Logs a message with level Debug.
	///----------------------------------------------------------------------------------------------------
	void LogDebug(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// Log:
	/// 	Logs a message with level Trace.
	///----------------------------------------------------------------------------------------------------
	void Log(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// LogMessage:
	/// 	Logs a message to a specific channel.
	///----------------------------------------------------------------------------------------------------
	void LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// LogMessageV:
	/// 	Logs a message to a specific channel with printf-style formatting.
	///----------------------------------------------------------------------------------------------------
	void LogMessageV(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs);

	///----------------------------------------------------------------------------------------------------
	/// LogMessageUnformatted:
	/// 	Logs an unformatted message to a specific channel.
	///----------------------------------------------------------------------------------------------------
	void LogMessageUnformatted(ELogLevel aLogLevel, std::string aChannel, const char* aMsg);
}

#endif
