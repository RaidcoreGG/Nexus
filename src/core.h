#ifndef CORE_H
#define CORE_H

#include <Windows.h>
#include <filesystem>

void PathSystemAppend(std::filesystem::path& aDestination, const char* aAppend);

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

std::string WStrToStr(const std::wstring& aWstring);
std::wstring StrToWStr(const std::string& aString);

const char* ConvertToUTF8(const char* multibyteStr);

std::vector<unsigned char> MD5(const unsigned char* data, size_t sz);
std::vector<unsigned char> MD5FromFile(const std::filesystem::path& aPath);

std::string GetBaseURL(const std::string& aUrl);
std::string GetEndpoint(const std::string& aUrl);
#endif
