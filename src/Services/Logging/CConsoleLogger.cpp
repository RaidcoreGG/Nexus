///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CConsoleLogger.cpp
/// Description  :  Custom logger to print to a console window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CConsoleLogger.h"

#include <Windows.h>
#include <iostream>
#include <iomanip>

HANDLE hConsole;
FILE* iobuf;

CConsoleLogger::CConsoleLogger(ELogLevel aLogLevel)
{
	LogLevel = aLogLevel;
	AllocConsole();
	freopen_s(&iobuf, "CONIN$", "r", stdin);
	freopen_s(&iobuf, "CONOUT$", "w", stderr);
	freopen_s(&iobuf, "CONOUT$", "w", stdout);
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

CConsoleLogger::~CConsoleLogger()
{
	FreeConsole();
}

void CConsoleLogger::LogMessage(LogEntry aLogEntry)
{
	const std::lock_guard<std::mutex> lock(Mutex);
	{
		switch (aLogEntry.LogLevel)
		{
			case ELogLevel::CRITICAL:    SetConsoleTextAttribute(hConsole, 12); break;
			case ELogLevel::WARNING:     SetConsoleTextAttribute(hConsole, 14); break;
			case ELogLevel::INFO:        SetConsoleTextAttribute(hConsole, 10); break;
			case ELogLevel::DEBUG:       SetConsoleTextAttribute(hConsole, 11); break;
			default:                     SetConsoleTextAttribute(hConsole, 7); break;
		}

		std::cout << aLogEntry.ToString();
	}
}
