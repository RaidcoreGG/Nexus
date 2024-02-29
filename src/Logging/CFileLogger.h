#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <fstream>
#include <filesystem>

#include "ILogger.h"

class CFileLogger : public virtual ILogger
{
	public:
		CFileLogger(ELogLevel aLogLevel, std::filesystem::path aPath);
		~CFileLogger();

		void LogMessage(LogEntry aLogEntry);

	private:
		std::ofstream File;
};

#endif