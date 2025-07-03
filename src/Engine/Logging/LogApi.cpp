///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogApi.cpp
/// Description  :  Provides logging functions and allows for custom logging implementations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogApi.h"

#include <cstdarg>

#include "Util/Time.h"

CLogApi::~CLogApi()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto it = this->Messages.begin(); it != this->Messages.end();)
	{
		LogMsg_t* msg = *it;

		delete msg;
		it = this->Messages.erase(it);
	}
}

void CLogApi::Register(ILogger* aLogger)
{
	if (!aLogger) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.push_back(aLogger);

	/* Replay all log messages. */
	for (LogMsg_t* msg : this->Messages)
	{
		/* Must match logger filter. */
		if (msg->Level <= aLogger->GetLogLevel())
		{
			aLogger->MsgProc(msg);
		}
	}
}

void CLogApi::Deregister(ILogger* aLogger)
{
	if (!aLogger) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find(this->Registry.begin(), this->Registry.end(), aLogger);

	this->Registry.erase(it);
}

void CLogApi::Trace(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogV(ELogLevel::TRACE, aChannel, aFmt, args);
	va_end(args);
}

void CLogApi::Critical(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogV(ELogLevel::CRITICAL, aChannel, aFmt, args);
	va_end(args);
}

void CLogApi::Warning(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogV(ELogLevel::WARNING, aChannel, aFmt, args);
	va_end(args);
}

void CLogApi::Info(const std::string& aChannel, const char* aFmt, ...)
{
	va_list args;
	va_start(args, aFmt);
	this->LogV(ELogLevel::INFO, aChannel, aFmt, args);
	va_end(args);
}

void CLogApi::Debug(const std::string& aChannel, const char* aFmt, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, aFmt);
	this->LogV(ELogLevel::DEBUG, aChannel, aFmt, args);
	va_end(args);
#endif
}

void CLogApi::Log(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...)
{
	/* Clean log level. */
	if (aLogLevel == ELogLevel::OFF || aLogLevel == ELogLevel::ALL)
	{
		aLogLevel = ELogLevel::TRACE;
	}

	va_list args;
	va_start(args, aFmt);
	this->LogV(aLogLevel, aChannel, aFmt, args);
	va_end(args);
}

void CLogApi::LogV(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, va_list aArgs)
{
	char buffer[4096]{};
	vsprintf_s(buffer, 4095, aFmt, aArgs); // 4096-1 for guaranteed null terminator

	this->LogUnformatted(aLogLevel, aChannel, &buffer[0]);
}

void CLogApi::LogUnformatted(ELogLevel aLogLevel, std::string aChannel, const char* aMsg)
{
	LogMsg_t* msg = nullptr;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	LogMsg_t* lastMsg = this->Messages.size() > 0 ? this->Messages.back() : nullptr;

	if (lastMsg && aMsg == lastMsg->Message && aLogLevel == lastMsg->Level)
	{
		msg = lastMsg;
		lastMsg->RepeatCount++;
	}
	else
	{
		msg = new LogMsg_t();
		msg->Level           = aLogLevel;
		msg->Time            = Time::GetTimestamp();
		msg->TimeMsPrecision = Time::GetMilliseconds();
		msg->Channel         = aChannel;
		msg->Message         = aMsg;

		/* store new log entry */
		this->Messages.push_back(msg);
	}

	/* Dispatch message. */
	for (ILogger* logger : this->Registry)
	{
		/* Must match logger filter. */
		if (msg->Level <= logger->GetLogLevel())
		{
			logger->MsgProc(msg);
		}
	}
}
