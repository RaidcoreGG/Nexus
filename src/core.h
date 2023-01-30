#ifndef CORE_H
#define CORE_H

#include <Windows.h>
#include <codecvt>
#include <locale>
#include <PathCch.h>

void PathCopyAndAppend(wchar_t* aSource, wchar_t* aDestination, const wchar_t* aAppend);

void PathSystemAppend(wchar_t* aDestination, const wchar_t* aAppend);

void PathGetDirectoryName(wchar_t* aSource, wchar_t* aDestination);

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

std::string WStrToStr(std::wstring& aWstring);

std::wstring StrToWStr(std::string& aString);

#endif