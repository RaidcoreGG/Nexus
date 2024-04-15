#ifndef CORE_H
#define CORE_H

#include <filesystem>
#include <string>
#include <vector>
#include <Windows.h>
#include "Loader/EUpdateProvider.h"

void PathSystemAppend(std::filesystem::path& aDestination, const char* aAppend);

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

std::string WStrToStr(const std::wstring& aWstring);
std::wstring StrToWStr(const std::string& aString);

namespace String
{
	std::string Replace(const std::string& aString, const std::string& aOld, const std::string& aNew, size_t aPosition = 0);
	bool Contains(const std::string& aString, const std::string& aStringFind);
	std::vector<std::string> Split(std::string aString, const std::string& aDelimiter, bool aKeepDelimiters = false);
}

const char* ConvertToUTF8(const char* multibyteStr);

std::vector<unsigned char> MD5(const unsigned char* data, size_t sz);
std::vector<unsigned char> MD5FromFile(const std::filesystem::path& aPath);
std::string MD5ToString(const std::vector<unsigned char>& aBytes);

EUpdateProvider GetProvider(const std::string& aUrl);
std::string GetBaseURL(const std::string& aUrl);
std::string GetEndpoint(const std::string& aUrl);
std::string GetQuery(const std::string& aEndpoint, const std::string& aParameters);
std::string Normalize(const std::string& aString);

namespace Base64
{
	std::string Encode(const unsigned char* src, size_t len);
	std::string Decode(const void* data, const size_t len);
}
#endif

long long Timestamp();
long long LastModifiedToTimestamp(const std::string& aLastModified);

bool CreateDirectoryRecursive(std::string const& dirName, std::error_code& err);

bool GetResource(HMODULE aModule, LPCSTR aResourceName, LPCSTR aResourceType, LPVOID* aOutLockedResource, DWORD* aOutResourceSize);
