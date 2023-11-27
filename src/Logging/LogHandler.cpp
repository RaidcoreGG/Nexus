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
			ConsoleLogger* cLog = new ConsoleLogger(ELogLevel::ALL);
			RegisterLogger(cLog);
		}

		FileLogger* fLog = new FileLogger(ELogLevel::ALL, Path::F_LOG);
		RegisterLogger(fLog);
	}

	void RegisterLogger(ILogger* aLogger)
	{
		Mutex.lock();
		{
			Registry.push_back(aLogger);
		}
		Mutex.unlock();
	}
	void UnregisterLogger(ILogger* aLogger)
	{
		Mutex.lock();
		{
			Registry.erase(std::remove(Registry.begin(), Registry.end(), aLogger), Registry.end());
		}
		Mutex.unlock();
	}

	/* Logging helper functions */
	void Log(std::string aChannel, const char* aFmt, ...)             { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::TRACE,    aChannel, aFmt, args); va_end(args); }
	void LogCritical(std::string aChannel, const char* aFmt, ...)     { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::CRITICAL, aChannel, aFmt, args); va_end(args); }
	void LogWarning(std::string aChannel, const char* aFmt, ...)      { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::WARNING,  aChannel, aFmt, args); va_end(args); }
	void LogInfo(std::string aChannel, const char* aFmt, ...)         { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::INFO,     aChannel, aFmt, args); va_end(args); }
	void LogDebug(std::string aChannel, const char* aFmt, ...)        { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::DEBUG,    aChannel, aFmt, args); va_end(args); }

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

		Mutex.lock();
		{
			if (std::find(Channels.begin(), Channels.end(), aChannel) == Channels.end())
			{
				Channels.push_back(aChannel);
			}
		}
		Mutex.unlock();

		char buffer[4096];
		vsprintf_s(buffer, 4096, aFmt, aArgs);

		entry.Message = std::string(&buffer[0], &buffer[strlen(buffer)]);

		Mutex.lock();
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
		Mutex.unlock();
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Mutex.lock();
		{
			for (ILogger* logger : Registry)
			{
				if (logger >= aStartAddress && logger <= aEndAddress)
				{
					Registry.erase(std::remove(Registry.begin(), Registry.end(), logger), Registry.end());
					refCounter++;
				}
			}
		}
		Mutex.unlock();

		return refCounter;
	}
}