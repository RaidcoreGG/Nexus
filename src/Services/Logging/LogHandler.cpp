///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogHandler.cpp
/// Description  :  Provides logging functions and allows for custom logging implementations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogHandler.h"

#include <ctime>
#include <cstdarg>
#include <algorithm>
#include <chrono>

#include "Shared.h"

#include "Util/Time.h"

namespace LogHandler
{
	void ADDONAPI_LogMessage(ELogLevel aLogLevel, const char* aStr)
	{
		Logger->LogMessageUnformatted(aLogLevel, "Addon", aStr);
	}

	void ADDONAPI_LogMessage2(ELogLevel aLogLevel, const char* aChannel, const char* aStr)
	{
		Logger->LogMessageUnformatted(aLogLevel, aChannel, aStr);
	}
}

void CLogHandler::RegisterLogger(ILogger* aLogger)
{
	if (!aLogger) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.push_back(aLogger);

	/* replay log messages */
	for (LogEntry* entry : this->LogEntries)
	{
		aLogger->LogMessage(entry);
	}
}

void CLogHandler::DeregisterLogger(ILogger* aLogger)
{
	if (!aLogger) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.erase(std::remove(this->Registry.begin(), this->Registry.end(), aLogger), this->Registry.end());
}

void CLogHandler::Trace(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogMessageV(ELogLevel::TRACE, aChannel, aFmt, args);
	va_end(args);
}

void CLogHandler::Critical(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogMessageV(ELogLevel::CRITICAL, aChannel, aFmt, args);
	va_end(args);
}

void CLogHandler::Warning(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogMessageV(ELogLevel::WARNING, aChannel, aFmt, args);
	va_end(args);
}

void CLogHandler::Info(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogMessageV(ELogLevel::INFO, aChannel, aFmt, args);
	va_end(args);
}

void CLogHandler::Debug(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogMessageV(ELogLevel::DEBUG, aChannel, aFmt, args);
	va_end(args);
}

void CLogHandler::LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogMessageV(aLogLevel, aChannel, aFmt, args);
	va_end(args);
}

void CLogHandler::LogMessageV(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs)
{
	char buffer[4096];
	vsprintf_s(buffer, 4095, aFmt, aArgs); // 4096-1 for \0 terminator

	this->LogMessageUnformatted(aLogLevel, aChannel, &buffer[0]);
}

void CLogHandler::LogMessageUnformatted(ELogLevel aLogLevel, std::string aChannel, const char* aMsg)
{
	LogEntry* entry = new LogEntry();
	entry->LogLevel = aLogLevel;
	entry->Timestamp = Time::GetTimestamp();
	entry->TimestampMilliseconds = Time::GetMilliseconds();
	entry->Channel = aChannel;
	entry->Message = aMsg;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* store log entries */
	this->LogEntries.push_back(entry);

	for (ILogger* logger : this->Registry)
	{
		/* send logged message to logger if message log level is lower than logger level */
		if (entry->LogLevel <= logger->GetLogLevel())
		{
			logger->LogMessage(entry);
		}
	}
}
