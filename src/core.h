#ifndef CORE_H
#define CORE_H

#include <Windows.h>
#include <codecvt>
#include <locale>
#include <PathCch.h>
#include <Shlwapi.h>

void PathCopyAndAppend(char* aSource, char* aDestination, const char* aAppend);

void PathSystemAppend(char* aDestination, const char* aAppend);

void PathGetDirectoryName(char* aSource, char* aDestination);

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

std::string WStrToStr(std::wstring& aWstring);

std::wstring StrToWStr(std::string& aString);

const char* ConvertToUTF8(const char* multibyteStr);

#endif
