#include "core.h"

#include <codecvt>
#include <locale>
#include <PathCch.h>
#include <Shlwapi.h>
#include <fstream>

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
	std::ifstream file(aPath, std::ios::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	char* buffer = new char[length];
	file.read(buffer, length);

	std::vector<unsigned char> md5 = MD5((const unsigned char*)buffer, length);

	delete[] buffer;

	file.close();

	return md5;
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