#include "LogHandler.h"

#include <ctime>
#include <thread>
#include <cstdarg>
#include <algorithm>
#include <chrono>

#include "State.h"
#include "Paths.h"

#include "ConsoleLogger.h"
#include "FileLogger.h"

namespace LogHandler
{
	std::mutex Mutex;
	std::vector<ILogger*> Registry;
	std::vector<LogEntry> LogEntries;
	std::vector<std::string> Channels;

	void Initialize()
	{
		/* setup default loggers */
		if (State::IsConsoleEnabled)
		{
			RegisterLogger(new ConsoleLogger(ELogLevel::ALL));
		}

		RegisterLogger(new FileLogger(ELogLevel::ALL, Path::F_LOG));
	}

	void RegisterLogger(ILogger* aLogger)
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		{
			Registry.push_back(aLogger);
		}

	}
	void UnregisterLogger(ILogger* aLogger)
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		{
			Registry.erase(std::remove(Registry.begin(), Registry.end(), aLogger), Registry.end());
		}
	}

	/* Logging helper functions */
	void Log(const std::string& aChannel, const char* aFmt, ...)             { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::TRACE,    aChannel, aFmt, args); va_end(args); }
	void LogCritical(const std::string& aChannel, const char* aFmt, ...)     { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::CRITICAL, aChannel, aFmt, args); va_end(args); }
	void LogWarning(const std::string& aChannel, const char* aFmt, ...)      { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::WARNING,  aChannel, aFmt, args); va_end(args); }
	void LogInfo(const std::string& aChannel, const char* aFmt, ...)         { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::INFO,     aChannel, aFmt, args); va_end(args); }
	void LogDebug(const std::string& aChannel, const char* aFmt, ...)        { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::DEBUG,    aChannel, aFmt, args); va_end(args); }

	/* Basic logging functions */
	void LogMessageA(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...)	{ va_list args; va_start(args, aFmt); LogMessage(aLogLevel, aChannel, aFmt, args); va_end(args); }
	void LogMessageAddon(ELogLevel aLogLevel, const char* aStr)							{ LogMessageA(aLogLevel, "Addon", aStr); }

	/* Logging internal functions */
	void LogMessage(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs)
	{
		LogEntry entry;
		entry.LogLevel = aLogLevel;
		entry.Timestamp = time(NULL);
		entry.Channel = aChannel;

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			if (std::find(Channels.begin(), Channels.end(), aChannel) == Channels.end())
			{
				Channels.push_back(aChannel);
			}
		}

		char buffer[4096];
		vsprintf_s(buffer, 4096, aFmt, aArgs);

		entry.Message = std::string(&buffer[0], &buffer[strlen(buffer)]);

		{
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
}