///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Resources.cpp
/// Description  :  Contains a variety of utility for embedded resources.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include <fstream>

#include "Resources.h"

namespace Resources
{
	bool Get(HMODULE aModule, LPCSTR aResourceName, LPCSTR aResourceType, LPVOID* aOutLockedResource, DWORD* aOutResourceSize)
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

	bool Unpack(HMODULE aModule, std::filesystem::path aPath, unsigned int aResourceID, std::string aResourceType, bool aIsBinary)
	{
		LPVOID res{}; DWORD sz{};
		Resources::Get(aModule, MAKEINTRESOURCE(aResourceID), aResourceType.c_str(), &res, &sz);

		try
		{
			if (std::filesystem::exists(aPath))
			{
				std::filesystem::remove(aPath);
			}

			std::ofstream file(aPath, aIsBinary ? std::ios::binary : 0);
			file.write((const char*)res, sz);
			file.close();
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}
