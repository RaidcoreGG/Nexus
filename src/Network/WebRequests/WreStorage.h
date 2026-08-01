///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreStorage.h
/// Description  :  HttpClient factory and storage.
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
	class ClientStorage
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		ClientStorage(Core::LogApi& aLogger);

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~ClientStorage() = default;

		///----------------------------------------------------------------------------------------------------
		/// GetHttpClient:
		/// 	Returns or creates a http client with the given base URL.
		///----------------------------------------------------------------------------------------------------
		Network::CHttpClient& GetHttpClient(std::string aURL, bool aDisableCache = false);

		private:
		Core::LogApi&                                                Logger;

		std::mutex                                                   HttpClientMutex;
		std::map<std::string, std::unique_ptr<Network::CHttpClient>> HttpClients;
	};
}
