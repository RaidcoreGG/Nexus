#ifndef CORE_H
#define CORE_H

#include <filesystem>
#include <string>
#include <vector>
#include <Windows.h>

void PathSystemAppend(std::filesystem::path& aDestination, const char* aAppend);

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

std::string WStrToStr(const std::wstring& aWstring);
std::wstring StrToWStr(const std::string& aString);

const char* ConvertToUTF8(const char* multibyteStr);

std::vector<unsigned char> MD5(const unsigned char* data, size_t sz);
std::vector<unsigned char> MD5FromFile(const std::filesystem::path& aPath);

std::string GetBaseURL(const std::string& aUrl);
std::string GetEndpoint(const std::string& aUrl);

namespace Base64
{
	std::string Encode(const unsigned char* src, size_t len);
	std::string Decode(const void* data, const size_t len);
}
#endif
