#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <iostream>
#include <fstream>

#include "ILogger.h"

class FileLogger : public virtual ILogger
{
    public:
        FileLogger(const char* aPath);
        ~FileLogger();

        void LogMessage(LogEntry aLogEntry);

    private:
        std::ofstream File;
};

#endif