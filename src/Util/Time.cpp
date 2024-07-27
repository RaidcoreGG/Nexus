///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Time.cpp
/// Description  :  Contains a variety of utility for time based functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include <chrono>
#include <iomanip>
#include <sstream>

#include "Time.h"

namespace Time
{
	long long GetTimestamp()
	{
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	int GetMilliseconds()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 1000;
	}

	long long LastModifiedToTimestamp(const std::string& aLastModified)
	{
		std::tm tm = {};
		std::istringstream ss(aLastModified);
		ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
		if (ss.fail())
		{
			return -1;
		}
		tm.tm_isdst = 0;
		std::time_t t = std::mktime(&tm);
		if (t == -1)
		{
			return -1;
		}

		return t;
	}

	int GetYear()
	{
		std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm local_tm = *localtime(&t);
		return local_tm.tm_year;
	}

	int GetMonth()
	{
		std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm local_tm = *localtime(&t);
		return local_tm.tm_mon + 1;
	}

	int GetDay()
	{
		std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm local_tm = *localtime(&t);
		return local_tm.tm_mday;
	}

	int GetWeekday()
	{
		std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm local_tm = *localtime(&t);
		return local_tm.tm_wday;
	}
}
