///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogConsole.cpp
/// Description  :  Logger implementation to print to a console window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogConsole.h"

#include <Windows.h>
#include <iostream>
#include <iomanip>

#include "LogConst.h"

HANDLE hConsole;
FILE* iobuf;

CConsoleLogger::CConsoleLogger(ELogLevel aLogLevel)
{
	this->SetLogLevel(aLogLevel);
	AllocConsole();
	freopen_s(&iobuf, "CONIN$", "r", stdin);
	freopen_s(&iobuf, "CONOUT$", "w", stderr);
	freopen_s(&iobuf, "CONOUT$", "w", stdout);
	SetConsoleOutputCP(CP_UTF8);
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

CConsoleLogger::~CConsoleLogger()
{
	FreeConsole();
}

void CConsoleLogger::MsgProc(const LogMsg_t* aLogEntry)
{
	if (aLogEntry->RepeatCount == 1)
	{
		switch (aLogEntry->Level)
		{
			case ELogLevel::CRITICAL:    SetConsoleTextAttribute(hConsole, 12); break;
			case ELogLevel::WARNING:     SetConsoleTextAttribute(hConsole, 14); break;
			case ELogLevel::INFO:        SetConsoleTextAttribute(hConsole, 10); break;
			case ELogLevel::DEBUG:       SetConsoleTextAttribute(hConsole, 11); break;
			default:                     SetConsoleTextAttribute(hConsole, 7); break;
		}

		std::cout << ToString(aLogEntry);
	}
}
