///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Resources.h
/// Description  :  Contains a variety of utility for embedded resources.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <filesystem>

///----------------------------------------------------------------------------------------------------
/// Resources Namespace
///----------------------------------------------------------------------------------------------------
namespace Resources
{
	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns true on success and writes aOutLockedResource and aOutResourceSize.
	///----------------------------------------------------------------------------------------------------
	bool Get(HMODULE aModule, LPCSTR aResourceName, LPCSTR aResourceType, LPVOID* aOutLockedResource, DWORD* aOutResourceSize);

	///----------------------------------------------------------------------------------------------------
	/// Unpack:
	/// 	Returns true if the resource was written to disk.
	///----------------------------------------------------------------------------------------------------
	bool Unpack(HMODULE aModule, std::filesystem::path aPath, unsigned int aResourceID, std::string aResourceType, bool aIsBinary = false);
}
