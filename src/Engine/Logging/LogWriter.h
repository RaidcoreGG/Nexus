///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogWriter.h
/// Description  :  Logger implementation to write to a file.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>

#include "LogBase.h"

///----------------------------------------------------------------------------------------------------
/// CFileLogger Class
///----------------------------------------------------------------------------------------------------
class CFileLogger : public virtual ILogger
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CFileLogger(ELogLevel aLogLevel, std::filesystem::path aPath, uint32_t aFlushIntervalMs = 1000);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CFileLogger();

	///----------------------------------------------------------------------------------------------------
	/// MsgProc:
	/// 	Message processing function.
	///----------------------------------------------------------------------------------------------------
	void MsgProc(const LogMsg_t* aLogEntry) override;

	private:
	std::ofstream           File;
	uint32_t                Interval;
	std::thread             WriterThread;
	bool                    IsRunning = false;

	std::mutex              Mutex;
	std::condition_variable ConVar;
	std::queue<LogMsg_t>    MsgQueue;

	///----------------------------------------------------------------------------------------------------
	/// Flush:
	/// 	Periodically flushes the filestream.
	///----------------------------------------------------------------------------------------------------
	void Flush();
};
