#include "core.h"

#pragma warning( push )
#pragma warning( disable : 4995)
void PathCopyAndAppend(char* aSource, char* aDestination, const char* aAppend)
{
	memcpy(aDestination, aSource, MAX_PATH);
	PathAppendA(aDestination, aAppend);
}

void PathSystemAppend(char* aDestination, const char* aAppend)
{
	GetSystemDirectoryA(aDestination, MAX_PATH);
	PathAppendA(aDestination, aAppend);
}
#pragma warning( pop ) 

void PathGetDirectoryName(char* aSource, char* aDestination)
{
	memcpy(aDestination, aSource, MAX_PATH);
	PathRemoveFileSpecA(aDestination);
}

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName)
{
	FARPROC* fp = (FARPROC*)aFunction;
	*fp = aModule ? GetProcAddress(aModule, aName) : 0;
	return (fp != 0);
}

std::string WStrToStr(std::wstring& aWstring)
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

std::wstring StrToWStr(std::string& aString)
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