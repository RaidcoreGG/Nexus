#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include <Windows.h>
#include <iostream>
#include <iomanip>

#include "ILogger.h"

class ConsoleLogger : public virtual ILogger
{
	public:
		ConsoleLogger(ELogLevel aLogLevel);
		~ConsoleLogger();

		void LogMessage(LogEntry aLogEntry);
};

#endif