#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "ILogger.h"

class CConsoleLogger : public virtual ILogger
{
	public:
		CConsoleLogger(ELogLevel aLogLevel);
		~CConsoleLogger();

		void LogMessage(LogEntry aLogEntry);
};

#endif