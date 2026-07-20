///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  NetContext.h
/// Description  :  Network context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "Core/Logging/LogApi.h"
#include "Network/Updater/Updater.h"
#include "Network/WebRequests/WreClient.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::Network Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::Network
{
	///----------------------------------------------------------------------------------------------------
	/// Context Class
	///----------------------------------------------------------------------------------------------------
	class Context
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		Context(Core::LogApi& aLogger);

		///----------------------------------------------------------------------------------------------------
		/// Shutdown:
		/// 	Shuts down the host context.
		///----------------------------------------------------------------------------------------------------
		void Shutdown();

		///----------------------------------------------------------------------------------------------------
		/// GetHttpClient:
		/// 	Returns or creates a http client with the given base URL.
		///----------------------------------------------------------------------------------------------------
		Network::CHttpClient& GetHttpClient(std::string aURL, bool aDisableCache = false);

		///----------------------------------------------------------------------------------------------------
		/// Updater:
		/// 	Returns the updater instance.
		///----------------------------------------------------------------------------------------------------
		Network::Updater& Updater();

		private:
		Core::LogApi& _Logger;

		std::unique_ptr<Network::Updater> _Updater{ nullptr };

		std::mutex                                   HttpClientMutex{};
		std::map<std::string, Network::CHttpClient*> HttpClients{};
	};
}
