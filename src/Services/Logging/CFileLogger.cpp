///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CFileLogger.cpp
/// Description  :  Custom logger to print to a log file.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CFileLogger.h"
#include <sstream>
#include <iomanip>

CFileLogger::CFileLogger(ELogLevel aLogLevel, std::filesystem::path aPath)
{
	rotate(aPath);
	LogLevel = aLogLevel;
	File.open(aPath, std::ios_base::out, SH_DENYWR);
}

CFileLogger::~CFileLogger()
{
	File.flush();
	File.close();
}

void CFileLogger::LogMessage(LogEntry aLogEntry)
{
	const std::lock_guard<std::mutex> lock(Mutex);

	File << aLogEntry.ToString(true, true, true);
	File.flush();
}

void CFileLogger::rotate(const std::filesystem::path& aPath) {
	namespace fs = std::filesystem;

	if (fs::exists(aPath)) {
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		std::stringstream ss {};
		ss << "_" << std::put_time(std::localtime(&now), "%Y%m%d-%H%M");
		fs::path p = aPath;
		p += ss.str();

		fs::rename(aPath, p);
	}
	// TODO: cleanup old logs. (Maybe have a configurable number of logs you keep)
	// Problem: Settings get loaded after logging gets initialized
}