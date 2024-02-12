#include "core.h"

#include <codecvt>
#include <locale>
#include <PathCch.h>
#include <Shlwapi.h>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "openssl/evp.h"
#include "openssl/md5.h"

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

namespace String
{
	std::string Replace(const std::string& aString, const std::string& aOld, const std::string& aNew, size_t aPosition)
	{
		std::string retStr = aString;
		if (aOld.empty())
		{
			return retStr;
		}

		size_t pos = aPosition;
		while ((pos = retStr.find(aOld, pos)) != std::string::npos)
		{
			retStr.replace(pos, aOld.length(), aNew);
			pos += aNew.length();
		}

		return retStr;
	}
	bool Contains(const std::string& aString, const std::string& aStringFind)
	{
		return aString.find(aStringFind) != std::string::npos;
	}
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

std::vector<unsigned char> MD5(const unsigned char* data, size_t sz)
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
std::vector<unsigned char> MD5FromFile(const std::filesystem::path& aPath)
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

	std::vector<unsigned char> md5 = MD5((const unsigned char*)buffer, length);

	delete[] buffer;

	file.close();

	return md5;
}
std::string MD5ToString(const std::vector<unsigned char>& aBytes)
{
	std::stringstream oss;
	for (size_t i = 0; i < aBytes.size(); i++)
	{
		oss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)aBytes[i];
	}
	std::string str = oss.str();
	return str;
}

EUpdateProvider GetProvider(const std::string& aUrl)
{
	if (String::Contains(aUrl, "raidcore.gg"))
	{
		return EUpdateProvider::Raidcore;
	}
	if (String::Contains(aUrl, "github.com"))
	{
		return EUpdateProvider::GitHub;
	}

	return EUpdateProvider::Direct;
}
std::string GetBaseURL(const std::string& aUrl)
{
	size_t httpIdx = aUrl.find("http://");
	size_t httpsIdx = aUrl.find("https://");

	size_t off = 0;
	if (httpIdx != std::string::npos)
	{
		off = httpIdx + 7; // 7 is length of "http://"
	}
	if (httpsIdx != std::string::npos)
	{
		off = httpsIdx + 8; // 8 is length of "https://"
	}

	size_t idx = aUrl.find('/', off);
	if (idx == std::string::npos)
	{
		return aUrl;
	}

	return aUrl.substr(0, idx);
}
std::string GetEndpoint(const std::string& aUrl)
{
	size_t httpIdx = aUrl.find("http://");
	size_t httpsIdx = aUrl.find("https://");

	size_t off = 0;
	if (httpIdx != std::string::npos)
	{
		off = httpIdx + 7; // 7 is length of "http://"
	}
	if (httpsIdx != std::string::npos)
	{
		off = httpsIdx + 8; // 8 is length of "https://"
	}

	size_t idx = aUrl.find('/', off);
	if (idx == std::string::npos)
	{
		return aUrl;
	}

	return aUrl.substr(idx);
}
std::string GetQuery(const std::string& aEndpoint, const std::string& aParameters)
{
	std::string rQuery;

	if (!aEndpoint.find("/", 0) == 0)
	{
		rQuery.append("/");
	}
	rQuery.append(aEndpoint);

	if (!aParameters.find("?", 0) == 0 && aParameters.length() > 0)
	{
		rQuery.append("?");
	}
	if (!aParameters.empty())
	{
		rQuery.append(aParameters);
	}

	return rQuery;
}

namespace Base64
{
	static const unsigned char base64_table[65] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::string Encode(const unsigned char* src, size_t len)
	{
		unsigned char* out, * pos;
		const unsigned char* end, * in;

		size_t olen;

		olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

		if (olen < len)
			return std::string(); /* integer overflow */

		std::string outStr;
		outStr.resize(olen);
		out = (unsigned char*)&outStr[0];

		end = src + len;
		in = src;
		pos = out;
		while (end - in >= 3) {
			*pos++ = base64_table[in[0] >> 2];
			*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
			*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
			*pos++ = base64_table[in[2] & 0x3f];
			in += 3;
		}

		if (end - in) {
			*pos++ = base64_table[in[0] >> 2];
			if (end - in == 1) {
				*pos++ = base64_table[(in[0] & 0x03) << 4];
				*pos++ = '=';
			}
			else {
				*pos++ = base64_table[((in[0] & 0x03) << 4) |
					(in[1] >> 4)];
				*pos++ = base64_table[(in[1] & 0x0f) << 2];
			}
			*pos++ = '=';
		}

		return outStr;
	}

	static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
	7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
	0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

	std::string Decode(const void* data, const size_t len)
	{
		unsigned char* p = (unsigned char*)data;
		int pad = len > 0 && (len % 4 || p[len - 1] == '=');
		const size_t L = ((len + 3) / 4 - pad) * 4;
		std::string str(L / 4 * 3 + pad, '\0');

		for (size_t i = 0, j = 0; i < L; i += 4)
		{
			int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
			str[j++] = n >> 16;
			str[j++] = n >> 8 & 0xFF;
			str[j++] = n & 0xFF;
		}
		if (pad)
		{
			int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
			str[str.size() - 1] = n >> 16;

			if (len > L + 2 && p[L + 2] != '=')
			{
				n |= B64index[p[L + 2]] << 6;
				str.push_back(n >> 8 & 0xFF);
			}
		}
		return str;
	}
}

long long Timestamp()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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