///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogConsole.cpp
/// Description  :  Logger implementation to print to a console window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogConsole.h"

#include <windows.h>
#include <iostream>

#include "LogConst.h"

CConsoleLogger::CConsoleLogger(ELogLevel aLogLevel)
{
	this->SetLogLevel(aLogLevel);
	AllocConsole();
	freopen_s(&this->IOBuf, "CONIN$", "r", stdin);
	freopen_s(&this->IOBuf, "CONOUT$", "w", stderr);
	freopen_s(&this->IOBuf, "CONOUT$", "w", stdout);
	SetConsoleOutputCP(CP_UTF8);
	this->ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
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
			case ELogLevel::CRITICAL: SetConsoleTextAttribute(this->ConsoleHandle, 12); break;
			case ELogLevel::WARNING:  SetConsoleTextAttribute(this->ConsoleHandle, 14); break;
			case ELogLevel::INFO:     SetConsoleTextAttribute(this->ConsoleHandle, 10); break;
			case ELogLevel::DEBUG:    SetConsoleTextAttribute(this->ConsoleHandle, 11); break;
			default:                  SetConsoleTextAttribute(this->ConsoleHandle, 7); break;
		}

		std::cout << ToString(aLogEntry);
	}
}
