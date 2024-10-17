///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Strings.cpp
/// Description  :  Contains a variety of string utility.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Strings.h"

#include <Windows.h>

#include <algorithm>
#include <cstdarg>
#include <stringapiset.h>

namespace String
{
#define MAX_STRING_FORMAT_LENGTH 4096

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

	std::vector<std::string> Split(const std::string& aString, const std::string& aDelimiter, bool aKeepDelimiters)
	{
		std::vector<std::string> parts;

		size_t pos = aString.find(aDelimiter);
		size_t initialPos = 0;

		while (pos != std::string::npos)
		{
			parts.push_back(aString.substr(initialPos, pos - initialPos));
			if (aKeepDelimiters)
			{
				parts.push_back(aDelimiter);
			}
			initialPos = pos + 1;

			pos = aString.find(aDelimiter, initialPos);
		}

		parts.push_back(aString.substr(initialPos, aString.length() - initialPos));

		return parts;
	}

	bool StartsWith(std::string aString, const std::string& aStringFind)
	{
		aString = aString.substr(0, aStringFind.length());

		if (aString == aStringFind)
		{
			return true;
		}

		return false;
	}

	bool EndsWith(std::string aString, const std::string& aStringFind)
	{
		aString = aString.substr(aString.length() - aStringFind.length(), aStringFind.length());

		if (aString == aStringFind)
		{
			return true;
		}

		return false;
	}

	std::string Format(std::string aFmt, ...)
	{
		va_list args;
		va_start(args, aFmt);
		char buffer[MAX_STRING_FORMAT_LENGTH];
		vsprintf_s(buffer, MAX_STRING_FORMAT_LENGTH - 1, aFmt.c_str(), args);
		va_end(args);

		return buffer;
	}

	std::string FormatByteSize(size_t aSize)
	{
		const char* sizes[] = { "Bytes", "KB", "MB", "GB" };
		float sz = (float)aSize;
		int order = 0;

		while (sz >= 1024 && order < 3)
		{
			order++;
			sz /= 1024;
		}

		return Format("%.2f %s", sz, sizes[order]);
	}

	std::string Normalize(const std::string& aString)
	{
		std::string ret;

		for (size_t i = 0; i < aString.length(); i++)
		{
			// alphanumeric
			if ((aString[i] >= 48 && aString[i] <= 57) ||
				(aString[i] >= 65 && aString[i] <= 90) ||
				(aString[i] >= 97 && aString[i] <= 122))
			{
				ret += aString[i];
			}
			else if (aString[i] == 32)
			{
				ret.append("_");
			}
		}

		return ret;
	}

	std::string ToLower(std::string aString)
	{
		std::transform(aString.begin(), aString.end(), aString.begin(), ::tolower);
		return aString;
	}

	std::string ToUpper(std::string aString)
	{
		std::transform(aString.begin(), aString.end(), aString.begin(), ::toupper);
		return aString;
	}

	std::string ToString(const std::wstring& aWstring)
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

	std::wstring ToWString(const std::string& aString)
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

	std::string ConvertMBToUTF8(std::string aString)
	{
		char* utf8Str = nullptr;

		int wideCharCount = MultiByteToWideChar(CP_ACP, 0, aString.c_str(), -1, NULL, 0);
		if (wideCharCount > 0)
		{
			wchar_t* wideCharBuff = new wchar_t[wideCharCount];
			MultiByteToWideChar(CP_ACP, 0, aString.c_str(), -1, wideCharBuff, wideCharCount);

			int utf8Count = WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, NULL, 0, NULL, NULL);
			if (utf8Count > 0)
			{
				utf8Str = new char[utf8Count];
				WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, utf8Str, utf8Count, NULL, NULL);
			}

			delete[] wideCharBuff;
		}

		return std::string(utf8Str ? utf8Str : "");
	}
}
