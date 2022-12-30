#include "core.h"

void PathCopyAndAppend(wchar_t* aSource, wchar_t* aDestination, const wchar_t* aAppend)
{
	memcpy(aDestination, aSource, MAX_PATH);
	PathCchAppend(aDestination, MAX_PATH, aAppend);
}

void PathSystemAppend(wchar_t* aDestination, const wchar_t* aAppend)
{
	GetSystemDirectoryW(aDestination, MAX_PATH);
	PathCchAppend(aDestination, MAX_PATH, aAppend);
}

void PathGetDirectoryName(wchar_t* aSource, wchar_t* aDestination)
{
	memcpy(aDestination, aSource, MAX_PATH);
	PathCchRemoveFileSpec(aDestination, MAX_PATH);
}

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName)
{
	FARPROC* fp = (FARPROC*)aFunction;
	*fp = aModule ? GetProcAddress(aModule, aName) : 0;
	return (fp != 0);
}

std::string WStrToStr(std::wstring& aWstring)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	return utf8_conv.to_bytes(aWstring);
}