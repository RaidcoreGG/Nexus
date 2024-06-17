#include "core.h"

#include <codecvt>
#include <locale>
#include <PathCch.h>
#include <Shlwapi.h>
#include <fstream>
#include <iomanip>
#include <sstream>

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName)
{
	FARPROC* fp = (FARPROC*)aFunction;
	*fp = aModule ? GetProcAddress(aModule, aName) : 0;
	return (*fp != 0);
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
