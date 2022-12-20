#include <iostream>
#include <iomanip>

#include "ConsoleLogger.h"
#include "windows.h"

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
    switch (aLogEntry.LogLevel)
    {
        case LogLevel::CRITICAL:    SetConsoleTextAttribute(hConsole, 12); break;
        case LogLevel::WARNING:     SetConsoleTextAttribute(hConsole, 14); break;
        case LogLevel::INFO:        SetConsoleTextAttribute(hConsole, 10); break;
        case LogLevel::DEBUG:       SetConsoleTextAttribute(hConsole, 11); break;
        default:                    SetConsoleTextAttribute(hConsole, 7); break;
    }
    
    std::wcout << aLogEntry.ToString();
}