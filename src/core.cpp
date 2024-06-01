#include "core.h"

#include <codecvt>
#include <locale>
#include <PathCch.h>
#include <Shlwapi.h>
#include <fstream>
#include <iomanip>
#include <sstream>

void PathSystemAppend(std::filesystem::path& aDestination, const char* aAppend)
{
	char* buff = new char[MAX_PATH];
	GetSystemDirectoryA(buff, MAX_PATH);
	aDestination = buff;
	aDestination.append(aAppend);
	delete[] buff;
}

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName)
{
	FARPROC* fp = (FARPROC*)aFunction;
	*fp = aModule ? GetProcAddress(aModule, aName) : 0;
	return (*fp != 0);
}

std::string WStrToStr(const std::wstring& aWstring)
{
	if (aWstring.empty())
	{
		return std::string();
	}

	int sz = WideCharToMultiByte(CP_ACP, 0, &aWstring[0], (int)aWstring.length(), 0, 0, 0, 0);
	std::string str(sz, 0);
	WideCharToMultiByte(CP_ACP, 0, &aWstring[0], (int)aWstring.length(), &str[0], sz, 0, 0);
	return str;
}
std::wstring StrToWStr(const std::string& aString)
{
	if (aString.empty())
	{
		return std::wstring();
	}

	int sz = MultiByteToWideChar(CP_ACP, 0, aString.c_str(), (int)aString.length(), 0, 0);
	std::wstring str(sz, 0);
	MultiByteToWideChar(CP_ACP, 0, aString.c_str(), (int)aString.length(), &str[0], (int)str.length());
	return str;
}

const char* ConvertToUTF8(const char* multibyteStr)
{
	char* utf8Str = nullptr;

	int wideCharCount = MultiByteToWideChar(CP_ACP, 0, multibyteStr, -1, NULL, 0);
	if (wideCharCount > 0)
	{
		wchar_t* wideCharBuff = new wchar_t[wideCharCount];
		MultiByteToWideChar(CP_ACP, 0, multibyteStr, -1, wideCharBuff, wideCharCount);

		int utf8Count = WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, NULL, 0, NULL, NULL);
		if (utf8Count > 0)
		{
			utf8Str = new char[utf8Count];
			WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, utf8Str, utf8Count, NULL, NULL);
		}

		delete[] wideCharBuff;
	}

	return utf8Str;
}

long long Timestamp()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
long long LastModifiedToTimestamp(const std::string& aLastModified)
{
	std::tm tm = {};
	std::istringstream ss(aLastModified);
	ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
	if (ss.fail())
	{
		return -1;
	}
	tm.tm_isdst = 0;
	std::time_t t = std::mktime(&tm);
	if (t == -1)
	{
		return -1;
	}

	return t;
}
// Returns:
//   true upon success.
//   false upon failure, and set the std::error_code & err accordingly.
bool CreateDirectoryRecursive(std::string const& dirName, std::error_code& err)
{
	err.clear();
	if (!std::filesystem::create_directories(dirName, err))
	{
		if (std::filesystem::exists(dirName))
		{
			// The folder already exists:
			err.clear();
			return true;
		}
		return false;
	}
	return true;
}

bool GetResource(HMODULE aModule, LPCSTR aResourceName, LPCSTR aResourceType, LPVOID* aOutLockedResource, DWORD* aOutResourceSize)
{
	HRSRC hRes = FindResourceA(aModule, aResourceName, aResourceType);
	if (!hRes)
	{
		return false;
	}

	HGLOBAL hLRes = LoadResource(aModule, hRes);
	if (!hLRes)
	{
		return false;
	}

	LPVOID pLRes = LockResource(hLRes);
	if (!pLRes)
	{
		return false;
	}

	DWORD dwResSz = SizeofResource(aModule, hRes);
	if (!dwResSz)
	{
		return false;
	}

	*aOutLockedResource = pLRes;
	*aOutResourceSize = dwResSz;

	return true;
}

std::filesystem::path GetUnclaimedPath(std::filesystem::path& aPath)
{
	std::filesystem::path probe = aPath;

	int i = 0;
	while (std::filesystem::exists(probe))
	{
		probe = aPath.string() + "_" + std::to_string(i);
		i++;
	}

	return probe;
}
