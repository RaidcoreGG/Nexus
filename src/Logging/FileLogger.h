#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <fstream>
#include <filesystem>

#include "ILogger.h"

class FileLogger : public virtual ILogger
{
	public:
		FileLogger(ELogLevel aLogLevel, std::filesystem::path aPath);
		~FileLogger();

		void LogMessage(LogEntry aLogEntry);

	private:
		std::ofstream File;
};

#endif