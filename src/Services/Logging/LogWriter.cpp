///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogWriter.cpp
/// Description  :  Logger implementation to write to a file.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogWriter.h"

#include <stdexcept>

#include "Util/Strings.h"
#include "LogConst.h"

CFileLogger::CFileLogger(ELogLevel aLogLevel, std::filesystem::path aPath)
{
	this->SetLogLevel(aLogLevel);
	this->File.open(aPath, std::ios_base::out, SH_DENYWR);

	/* TODO: Add exception handling. */
	//if (!this->File.is_open())
	//{
	//	throw std::runtime_error(String::Format("Failed to open \"%s\" for logging.", aPath.string()).c_str());
	//}
}

CFileLogger::~CFileLogger()
{
	this->File.flush();
	this->File.close();
}

void CFileLogger::MsgProc(const LogMsg_t* aLogEntry)
{
	this->File << ToString(aLogEntry);

	/* TODO: Add threading and periodic flushing for performance. */
	this->File.flush();
}
