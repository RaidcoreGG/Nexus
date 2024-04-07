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

namespace LogHandler
{
	void ADDONAPI_LogMessage(ELogLevel aLogLevel, const char* aStr)
	{
		LogMessageUnformatted(aLogLevel, "Addon", aStr);
	}
	void ADDONAPI_LogMessage2(ELogLevel aLogLevel, const char* aChannel, const char* aStr)
	{
		LogMessageUnformatted(aLogLevel, aChannel, aStr);
	}
}

namespace LogHandler
{
	std::mutex					Mutex;
	std::vector<ILogger*>		Registry;
	std::vector<LogEntry>		LogEntries;
	std::vector<std::string>	Channels;

	void RegisterLogger(ILogger* aLogger)
	{
		if (nullptr == aLogger)
		{
			return;
		}

		const std::lock_guard<std::mutex> lock(Mutex);

		Registry.push_back(aLogger);
	}
	void DeregisterLogger(ILogger* aLogger)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		Registry.erase(std::remove(Registry.begin(), Registry.end(), aLogger), Registry.end());
	}

	void Log(const std::string& aChannel, const char* aFmt, ...)			{ va_list args; va_start(args, aFmt);   LogMessageV(ELogLevel::TRACE,    aChannel, aFmt, args); va_end(args); }
	void LogCritical(const std::string& aChannel, const char* aFmt, ...)	{ va_list args; va_start(args, aFmt);   LogMessageV(ELogLevel::CRITICAL, aChannel, aFmt, args); va_end(args); }
	void LogWarning(const std::string& aChannel, const char* aFmt, ...)		{ va_list args; va_start(args, aFmt);   LogMessageV(ELogLevel::WARNING,  aChannel, aFmt, args); va_end(args); }
	void LogInfo(const std::string& aChannel, const char* aFmt, ...)		{ va_list args; va_start(args, aFmt);   LogMessageV(ELogLevel::INFO,     aChannel, aFmt, args); va_end(args); }
	void LogDebug(const std::string& aChannel, const char* aFmt, ...)		{ va_list args; va_start(args, aFmt);   LogMessageV(ELogLevel::DEBUG,    aChannel, aFmt, args); va_end(args); }

	void LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...)
	{
		va_list args;
		va_start(args, aFmt);
		LogMessageV(aLogLevel, aChannel, aFmt, args);
		va_end(args);
	}
	
	void LogMessageV(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs)
	{
		char buffer[4096];
		vsprintf_s(buffer, 4095, aFmt, aArgs); // 4096-1 for \0 terminator

		LogMessageUnformatted(aLogLevel, aChannel, &buffer[0]);
	}

	void LogMessageUnformatted(ELogLevel aLogLevel, std::string aChannel, const char* aMsg)
	{
		LogEntry entry;
		entry.LogLevel = aLogLevel;
		entry.Timestamp = time(NULL);
		entry.Channel = aChannel;
		entry.Message = aMsg;

		const std::lock_guard<std::mutex> lock(Mutex);
		
		if (std::find(Channels.begin(), Channels.end(), aChannel) == Channels.end())
		{
			Channels.push_back(aChannel);
		}

		LogEntries.push_back(entry);

		for (ILogger* logger : Registry)
		{
			/* send logged message to logger if message log level is lower than logger level */
			if (entry.LogLevel <= logger->GetLogLevel())
			{
				logger->LogMessage(entry);
			}
		}
	}
}
