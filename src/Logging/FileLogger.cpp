#include "FileLogger.h"
#include <sstream>
#include <iomanip>

FileLogger::FileLogger(const char* aPath)
{
    File.open(aPath);

    unsigned long long timestmap = time(0);

    struct tm timeinfo;
    localtime_s(&timeinfo, (time_t*)&timestmap);

    std::wstringstream oss;
    oss << std::put_time(&timeinfo, L"%Y-%m-%d %H:%M:%S");

    File << oss.str() << " : Log session start." << std::endl;
}

FileLogger::~FileLogger()
{
    unsigned long long timestmap = time(0);

    struct tm timeinfo;
    localtime_s(&timeinfo, (time_t*)&timestmap);

    std::wstringstream oss;
    oss << std::put_time(&timeinfo, L"%Y-%m-%d %H:%M:%S");

    File << oss.str() << " : Log session end." << std::endl;
    File << std::endl;
    File.close();
}

void FileLogger::LogMessage(LogEntry aLogEntry)
{
    MessageMutex.lock();

    File << aLogEntry.ToString();
    File.flush();

    MessageMutex.unlock();
}