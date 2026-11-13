///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogChannel.h
/// Description  :  Interface binding a component to a log channel.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "LogApi.h"

///----------------------------------------------------------------------------------------------------
/// ILogChannel Interface
/// 	Convenience interface to directly log to a channel without passing it per call.
///----------------------------------------------------------------------------------------------------
class ILogChannel
{
	public:
	inline const std::string& GetLogChannel() const
	{
		return this->LogChannel;
	}

	protected:
	ILogChannel(std::string aChannel) : LogChannel(aChannel) {}

	template <typename... TArgs>
	void LogCritical(const char* aFmt, TArgs... aArgs) const
	{
		::LogCritical(this->LogChannel, aFmt, aArgs...);
	}

	template <typename... TArgs>
	void LogWarning(const char* aFmt, TArgs... aArgs) const
	{
		::LogWarning(this->LogChannel, aFmt, aArgs...);
	}

	template <typename... TArgs>
	void LogInfo(const char* aFmt, TArgs... aArgs) const
	{
		::LogInfo(this->LogChannel, aFmt, aArgs...);
	}

	template <typename... TArgs>
	void LogDebug(const char* aFmt, TArgs... aArgs) const
	{
		::LogDebug(this->LogChannel, aFmt, aArgs...);
	}

	template <typename... TArgs>
	void LogTrace(const char* aFmt, TArgs... aArgs) const
	{
		::LogTrace(this->LogChannel, aFmt, aArgs...);
	}

	private:
	std::string LogChannel;
};
