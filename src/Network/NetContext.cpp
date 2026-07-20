///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  NetContext.cpp
/// Description  :  Network context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "NetContext.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

#include "Core/Logging/LogApi.h"
#include "Updater/Updater.h"
#include "Util/Strings.h"
#include "Util/Url.h"
#include "WebRequests/WreClient.h"

namespace Raidcore::Nexus::Network
{
	Context::Context(Core::LogApi& aLogger, std::filesystem::path aCommonDir)
		: _Logger(aLogger)
		, HttpCacheDir(aCommonDir)
	{
		this->_Updater = std::make_unique<Network::Updater>(&this->_Logger);
	}

	void Context::Shutdown()
	{
		this->_Updater.reset();

		const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

		for (auto it = this->HttpClients.begin(); it != this->HttpClients.end();)
		{
			/* Deallocate client. */
			delete it->second;

			/* Erase entry. */
			it = this->HttpClients.erase(it);
		}
	}

	Network::CHttpClient& Context::GetHttpClient(std::string aURL, bool aDisableCache)
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

		Network::CHttpClient* client = nullptr;

		if (aDisableCache)
		{
			client = new Network::CHttpClient(&this->_Logger, baseurl);
		}
		else
		{
			std::filesystem::path cachedir = this->HttpCacheDir / baseurl_noprotocol;
			uint32_t cacheLifetime = 30 * 60; // 30 minutes

			if (baseurl == "https://api.raidcore.gg")
			{
				cacheLifetime = 5 * 60; // 5 minutes
			}
			else if (baseurl == "https://api.github.com")
			{
				cacheLifetime = 60 * 60; // 60 minutes
			}

			client = new Network::CHttpClient(&this->_Logger, baseurl, cachedir, cacheLifetime);
		}

		this->HttpClients.emplace(baseurl_noprotocol, client);

		return *client;
	}

	Network::Updater& Context::Updater()
	{
		return *this->_Updater;
	}
}
