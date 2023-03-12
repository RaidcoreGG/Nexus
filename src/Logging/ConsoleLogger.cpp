#include "ConsoleLogger.h"

HANDLE hConsole;
FILE* iobuf;

ConsoleLogger::ConsoleLogger()
{
	AllocConsole();
	freopen_s(&iobuf, "CONIN$", "r", stdin);
	freopen_s(&iobuf, "CONOUT$", "w", stderr);
	freopen_s(&iobuf, "CONOUT$", "w", stdout);
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

ConsoleLogger::~ConsoleLogger()
{
	FreeConsole();
}

void ConsoleLogger::LogMessage(LogEntry aLogEntry)
{
	MessageMutex.lock();

	switch (aLogEntry.LogLevel)
	{
		case ELogLevel::CRITICAL:    SetConsoleTextAttribute(hConsole, 12); break;
		case ELogLevel::WARNING:     SetConsoleTextAttribute(hConsole, 14); break;
		case ELogLevel::INFO:        SetConsoleTextAttribute(hConsole, 10); break;
		case ELogLevel::DEBUG:       SetConsoleTextAttribute(hConsole, 11); break;
		default:                     SetConsoleTextAttribute(hConsole, 7); break;
	}
	
	std::cout << aLogEntry.ToString();

	MessageMutex.unlock();
}