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
