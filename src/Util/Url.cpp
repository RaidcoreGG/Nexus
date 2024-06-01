///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Url.cpp
/// Description  :  Contains a variety of utility for URLs.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Url.h"

#include "Util/Strings.h"

namespace URL
{
	std::string GetBase(const std::string& aUrl)
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

	std::string GetFilename(const std::string& aUrl)
	{
		size_t lastSlashPos = aUrl.find_last_of('/');
		std::string filename = aUrl.substr(lastSlashPos + 1);
		size_t extPos = filename.find_last_of(".");
		if (extPos != std::string::npos)
		{
			filename = filename.substr(0, extPos);
		}
		else
		{
			filename = String::Normalize(filename);
		}

		return filename;
	}

}
