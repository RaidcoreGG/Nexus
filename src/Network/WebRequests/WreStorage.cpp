///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreStorage.cpp
/// Description  :  HttpClient factory and storage implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "WreStorage.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

#include "Core/Logging/LogApi.h"
#include "Index/IdxEnum.h"
#include "Index/Index.h"
#include "Network/Updater/Updater.h"
#include "Util/Strings.h"
#include "Util/Url.h"
#include "WreClient.h"

namespace Raidcore::Nexus::Network
{
	ClientStorage::ClientStorage(Core::LogApi& aLogger)
		: Logger(aLogger)
	{}

	Network::CHttpClient& ClientStorage::GetHttpClient(std::string aURL, bool aDisableCache)
	{
		const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

		std::string baseurl = URL::GetBase(aURL);

		std::string baseurl_noprotocol = baseurl;
		baseurl_noprotocol = String::Replace(baseurl_noprotocol, "http://", "");
		baseurl_noprotocol = String::Replace(baseurl_noprotocol, "https://", "");

		auto it = this->HttpClients.find(baseurl_noprotocol);

		if (it != this->HttpClients.end())
		{
			return *it->second;
		}

		std::unique_ptr<Network::CHttpClient> client = nullptr;

		if (aDisableCache)
		{
			client = std::make_unique<Network::CHttpClient>(&this->Logger, baseurl);
		}
		else
		{
			std::filesystem::path cachedir = Index(EPath::DIR_COMMON) / baseurl_noprotocol;
			uint32_t cacheLifetime = 30 * 60; // 30 minutes

			if (baseurl == "https://api.raidcore.gg")
			{
				cacheLifetime = 5 * 60; // 5 minutes
			}
			else if (baseurl == "https://api.github.com")
			{
				cacheLifetime = 60 * 60; // 60 minutes
			}

			client = std::make_unique<Network::CHttpClient>(&this->Logger, baseurl, cachedir, cacheLifetime);
		}

		this->HttpClients.emplace(baseurl_noprotocol, std::move(client));

		return *this->HttpClients.at(baseurl_noprotocol).get();
	}
}
