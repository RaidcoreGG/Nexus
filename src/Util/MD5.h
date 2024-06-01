///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MD5.h
/// Description  :  Contains a variety of utility for MD5 hashes.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MD5_H
#define MD5_H

#include <vector>
#include <string>
#include <filesystem>

#include "httplib/httplib.h"

///----------------------------------------------------------------------------------------------------
/// MD5 Namespace
///----------------------------------------------------------------------------------------------------
namespace MD5Util
{
	///----------------------------------------------------------------------------------------------------
	/// FromMemory:
	/// 	Creates an MD5 hash from a buffer.
	///----------------------------------------------------------------------------------------------------
	std::vector<unsigned char> FromMemory(const unsigned char* aData, size_t aSize);

	///----------------------------------------------------------------------------------------------------
	/// FromFile:
	/// 	Creates an MD5 hash from a file.
	///----------------------------------------------------------------------------------------------------
	std::vector<unsigned char> FromFile(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// FromRemoteURL:
	/// 	Reads an MD5 hash from a remote URL.
	/// 	(It is expected that the remote URL contains an MD5 hash, this is simply read out.
	///----------------------------------------------------------------------------------------------------
	std::vector<unsigned char> FromRemoteURL(httplib::Client& aClient, std::string& aEndpoint);

	///----------------------------------------------------------------------------------------------------
	/// FromRemoteURL:
	/// 	Reads an MD5 hash from a remote URL.
	/// 	(It is expected that the remote URL contains an MD5 hash, this is simply read out.
	///----------------------------------------------------------------------------------------------------
	std::vector<unsigned char> FromRemoteURL(std::string& aBaseUrl, std::string& aEndpoint, bool aEnableSSL = true);

	///----------------------------------------------------------------------------------------------------
	/// ToString:
	/// 	Converts an MD5 hash to a hex string.
	///----------------------------------------------------------------------------------------------------
	std::string ToString(const std::vector<unsigned char>& aBytes);
}

#endif
