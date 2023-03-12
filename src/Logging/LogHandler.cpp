#include "LogHandler.h"

namespace LogHandler
{
	std::mutex Mutex;
	std::vector<ILogger*> Registry;
	std::vector<LogEntry> LogEntries;

	bool IsRunning = false;
	std::thread LoggingThread;
	std::vector<LogEntry> QueuedMessages;

	void Initialize()
	{
		IsRunning = true;
		LoggingThread = std::thread(ProcessQueue);
		LoggingThread.detach();
	}

	void RegisterLogger(ILogger* aLogger)
	{
		Mutex.lock();

		Registry.push_back(aLogger);

		Mutex.unlock();
	}
	void UnregisterLogger(ILogger* aLogger)
	{
		Mutex.lock();

		Registry.erase(std::remove(Registry.begin(), Registry.end(), aLogger), Registry.end());

		Mutex.unlock();
	}

	/* Logging helper functions */
	void Log(const char* aFmt, ...)             { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::TRACE,    aFmt, args); va_end(args); }
	void LogCritical(const char* aFmt, ...)     { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::CRITICAL, aFmt, args); va_end(args); }
	void LogWarning(const char* aFmt, ...)      { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::WARNING,  aFmt, args); va_end(args); }
	void LogInfo(const char* aFmt, ...)         { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::INFO,     aFmt, args); va_end(args); }
	void LogDebug(const char* aFmt, ...)        { va_list args; va_start(args, aFmt);   LogMessage(ELogLevel::DEBUG,    aFmt, args); va_end(args); }

	/* Basic logging functions */
	void LogMessageA(ELogLevel aLogLevel, const char* aFmt, ...)    { va_list args; va_start(args, aFmt); LogMessage(aLogLevel, aFmt, args); va_end(args); }
	
	void LogMessage(ELogLevel aLogLevel, const char* aFmt, va_list aArgs)
	{
		LogEntry entry;
		entry.LogLevel = aLogLevel;
		entry.Timestamp = time(NULL);

		char buffer[4096];
		vsprintf_s(buffer, 4096, aFmt, aArgs);

		entry.Message = std::string(&buffer[0], &buffer[strlen(buffer)]);

		Mutex.lock();
		QueuedMessages.push_back(entry);
		Mutex.unlock();
	}
	void ProcessQueue()
	{
		for (;;)
		{
			if (!IsRunning) { return; }

			while (QueuedMessages.size() > 0)
			{
				Mutex.lock();
				LogEntry& entry = QueuedMessages.front();

				for (ILogger* logger : Registry)
				{
					ELogLevel level = logger->GetLogLevel();

					/* send logged message to logger if message log level is lower than logger level */
					if (entry.LogLevel <= level)
					{
						logger->LogMessage(entry);
					}
				}

				LogEntries.push_back(entry);
				QueuedMessages.erase(QueuedMessages.begin());
				Mutex.unlock();
			}
		}
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Mutex.lock();
		for (ILogger* logger : Registry)
		{
			if (logger >= aStartAddress && logger <= aEndAddress)
			{
				UnregisterLogger(logger);
				refCounter++;
			}
		}
		Mutex.unlock();

		return refCounter;
	}
}