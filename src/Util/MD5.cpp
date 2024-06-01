///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MD5.cpp
/// Description  :  Contains a variety of utility for MD5 hashes.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "MD5.h"

#include "openssl/evp.h"
#include "openssl/md5.h"

namespace MD5Util
{
	std::vector<unsigned char> FromMemory(const unsigned char* data, size_t sz)
	{
		std::vector<unsigned char> md(MD5_DIGEST_LENGTH, 0);

		EVP_MD_CTX* ctx = EVP_MD_CTX_create();
		EVP_MD_CTX_init(ctx);
		EVP_DigestInit_ex(ctx, EVP_md5(), NULL);
		EVP_DigestUpdate(ctx, data, sz);
		EVP_DigestFinal_ex(ctx, md.data(), NULL);
		EVP_MD_CTX_destroy(ctx);

		return md;
	}

	std::vector<unsigned char> FromFile(const std::filesystem::path& aPath)
	{
		std::filesystem::path path = aPath;
		if (std::filesystem::is_symlink(aPath))
		{
			path = std::filesystem::read_symlink(aPath);
		}

		std::ifstream file(path, std::ios::binary);

		if (!file || !file.is_open())
		{
			return std::vector<unsigned char>();
		}

		file.seekg(0, std::ios::end);
		size_t length = file.tellg();
		file.seekg(0, std::ios::beg);

		if (length == 0)
		{
			return std::vector<unsigned char>();
		}

		char* buffer = new char[length];
		file.read(buffer, length);

		std::vector<unsigned char> md5 = FromMemory((const unsigned char*)buffer, length);

		delete[] buffer;

		file.close();

		return md5;
	}

	std::vector<unsigned char> FromRemoteURL(httplib::Client& aClient, std::string& aEndpoint)
	{
		std::vector<unsigned char> md5remote;

		auto resultMd5Req = aClient.Get(aEndpoint, [&](const char* data, size_t data_length) {
			for (size_t i = 0; i < data_length; i += 2)
			{
				if (md5remote.size() > MD5_DIGEST_LENGTH)
				{
					break; // this is not md5, we got more bytes than 16 -> abort
				}

				std::string str{};
				str += data[i];
				str += data[i + 1];

				unsigned char byte = (unsigned char)strtol(str.c_str(), NULL, 16);

				md5remote.push_back(byte);
			}
			return true;
			});

		/* there was a byte too much, but if it's a null terminator we just pop it */
		if (md5remote.size() == MD5_DIGEST_LENGTH + 1)
		{
			if (md5remote[16] == 0x0)
			{
				md5remote.pop_back();
			}
		}

		/* the read out bytes are not MD5 because too many bytes were read */
		if (md5remote.size() > MD5_DIGEST_LENGTH)
		{
			return {};
		}

		if (!resultMd5Req || resultMd5Req->status != 200)
		{
			return {};
		}

		return md5remote;
	}

	std::vector<unsigned char> FromRemoteURL(std::string& aBaseUrl, std::string& aEndpoint, bool aEnableSSL)
	{
		httplib::Client client(aBaseUrl);
		client.enable_server_certificate_verification(aEnableSSL);

		return FromRemoteURL(client, aEndpoint);
	}

	std::string ToString(const std::vector<unsigned char>& aBytes)
	{
		std::stringstream oss;
		for (size_t i = 0; i < aBytes.size(); i++)
		{
			oss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)aBytes[i];
		}
		std::string str = oss.str();
		return str;
	}
}
