#ifndef CORE_H
#define CORE_H

#include <Windows.h>
#include <codecvt>
#include <locale>

BOOL FindFunction(HMODULE, LPVOID, LPCSTR);

std::string WStrToStr(std::wstring& wide_string);
std::wstring StrToWstr(std::string& string);

#endif
