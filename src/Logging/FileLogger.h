#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <iostream>
#include <fstream>

#include "ILogger.h"

class FileLogger : public virtual ILogger
{
    public:
        FileLogger(const char*);
        ~FileLogger();

        void LogMessage(LogEntry);

    private:
        std::wofstream File;
};

#endif