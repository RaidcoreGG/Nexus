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
/// CLogHandler Class
///----------------------------------------------------------------------------------------------------
class CLogHandler
{
public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLogHandler() = default;
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLogHandler() = default;

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
	/// Critical:
	/// 	Logs a message with level Critical.
	///----------------------------------------------------------------------------------------------------
	void Critical(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// Warning:
	/// 	Logs a message with level Warning.
	///----------------------------------------------------------------------------------------------------
	void Warning(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// Info:
	/// 	Logs a message with level Info.
	///----------------------------------------------------------------------------------------------------
	void Info(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// Debug:
	/// 	Logs a message with level Debug.
	///----------------------------------------------------------------------------------------------------
	void Debug(const std::string& aChannel, const char* aFmt, ...);

	///----------------------------------------------------------------------------------------------------
	/// Log:
	/// 	Logs a message with level Trace.
	///----------------------------------------------------------------------------------------------------
	void Trace(const std::string& aChannel, const char* aFmt, ...);

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

private:
	std::mutex             Mutex;
	std::vector<ILogger*>  Registry;
	std::vector<LogEntry*> LogEntries;

	std::string            LastMessage;
	ELogLevel              LastMessageLevel;
};

#endif
