///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  BuildInfoService.h
/// Description  :  Guild Wars 2 build info service.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <mutex>

#include "Core/Logging/LogApi.h"
#include "Network/WebRequests/WreClient.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2 Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2
{
	///----------------------------------------------------------------------------------------------------
	/// BuildInfoService Class
	///----------------------------------------------------------------------------------------------------
	class BuildInfoService
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		BuildInfoService(
			Network::CHttpClient& aArenaNetAssetCDN,
			Core::LogApi&        aLogger
		);

		///----------------------------------------------------------------------------------------------------
		/// Build:
		/// 	Returns the current game build.
		///----------------------------------------------------------------------------------------------------
		uint32_t Build();

		private:
		Network::CHttpClient& _ArenaNetAssetCDN;
		Core::LogApi&        _Logger;

		std::mutex            Mutex{};
		uint32_t              _Build{ 0 };
	};
}
