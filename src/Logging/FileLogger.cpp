#include "FileLogger.h"
#include <sstream>
#include <iomanip>

FileLogger::FileLogger(const char* aPath)
{
    mFile.open(aPath, std::ios_base::app);

    unsigned long long timestmap = time(0);

    struct tm timeinfo;
    localtime_s(&timeinfo, (time_t*)&timestmap);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");

    mFile << oss.str() << " : Log session start." << std::endl;
}

FileLogger::~FileLogger()
{
    unsigned long long timestmap = time(0);

    struct tm timeinfo;
    localtime_s(&timeinfo, (time_t*)&timestmap);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");

    mFile << oss.str() << " : Log session end." << std::endl;
    mFile << std::endl;
    mFile.close();
}

void FileLogger::LogMessage(LogEntry aLogEntry)
{
    mFile << aLogEntry.ToString();
    mFile.flush();
}