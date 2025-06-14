///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogWriter.cpp
/// Description  :  Logger implementation to write to a file.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogWriter.h"

#include <windows.h>

#include "LogConst.h"

CFileLogger::CFileLogger(ELogLevel aLogLevel, std::filesystem::path aPath, uint32_t aFlushIntervalMs)
{
	this->SetLogLevel(aLogLevel);
	this->File.open(aPath, std::ios_base::out, SH_DENYWR);

	this->Interval = aFlushIntervalMs > 0 ? aFlushIntervalMs : 1000;
	
	if (this->File.is_open())
	{
		this->IsRunning = true;
		this->WriterThread = std::thread(&CFileLogger::Flush, this);
	}
}

CFileLogger::~CFileLogger()
{
	this->IsRunning = false;

	if (this->WriterThread.joinable())
	{
		this->WriterThread.join();
	}

	if (this->File.is_open())
	{
		this->File.flush();
		this->File.close();
	}
}

void CFileLogger::MsgProc(const LogMsg_t* aLogEntry)
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->Mutex);
	this->MsgQueue.push(*aLogEntry);
	this->ConVar.notify_one();
}

void CFileLogger::Flush()
{
	while (this->IsRunning)
	{
		std::unique_lock<std::mutex> lock(this->Mutex);
		this->ConVar.wait_for(lock, std::chrono::milliseconds(this->Interval), [this] {
			return !this->MsgQueue.empty() || !this->IsRunning;
		});

		while (!this->MsgQueue.empty())
		{
			this->File << ToString(&this->MsgQueue.front());
			this->MsgQueue.pop();
		}
		this->File.flush();
	}
}
