#ifndef CORE_H
#define CORE_H

#include <Windows.h>
#include <codecvt>
#include <locale>
#include <PathCch.h>
#include <Shlwapi.h>
#include <filesystem>

void PathSystemAppend(std::filesystem::path& aDestination, const char* aAppend);

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

std::string WStrToStr(std::wstring& aWstring);

std::wstring StrToWStr(std::string& aString);

const char* ConvertToUTF8(const char* multibyteStr);

#endif
