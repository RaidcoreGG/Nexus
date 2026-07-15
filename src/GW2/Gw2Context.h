///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Gw2Context.h
/// Description  :  Guild Wars 2 game context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <memory>
#include <utility>

#include "Core/DataLink/DlApi.h"
#include "Host/Events/EvtApi.h"
#include "Core/Logging/LogApi.h"
#include "Network/WebRequests/WreClient.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/BuildInfo/BuildInfoService.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "Graphics/GrContext.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2 Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2
{
	constexpr const char* LOG_CHANNEL = "GW2";

	///----------------------------------------------------------------------------------------------------
	/// Context Class
	///----------------------------------------------------------------------------------------------------
	class Context
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		Context(
			CDataLinkApi&          aDataLink,
			Host::EventApi&        aEventApi,
			CLogApi&               aLogger,
			Platform::RawInputApi& aRawInputApi,
			HWND                   aGameWindow,
			Network::CHttpClient&  aArenaNetAssetCDN,
			std::filesystem::path  aGameBindsPath
		);

		///----------------------------------------------------------------------------------------------------
		/// Initialize:
		/// 	Initializes the Guild Wars 2 game context.
		///----------------------------------------------------------------------------------------------------
		void Initialize();

		///----------------------------------------------------------------------------------------------------
		/// Shutdown:
		/// 	Shuts down the game context.
		///----------------------------------------------------------------------------------------------------
		void Shutdown();

		///----------------------------------------------------------------------------------------------------
		/// Arcdps:
		/// 	Returns the ArcDPS API.
		///----------------------------------------------------------------------------------------------------
		ArcdpsApi& Arcdps();

		///----------------------------------------------------------------------------------------------------
		/// Build:
		/// 	Returns the current game build information.
		///----------------------------------------------------------------------------------------------------
		BuildInfoService& BuildInfo();

		///----------------------------------------------------------------------------------------------------
		/// GameBinds:
		/// 	Returns the GameBinds API.
		///----------------------------------------------------------------------------------------------------
		GameBindsApi& GameBinds();

		///----------------------------------------------------------------------------------------------------
		/// Mumble:
		/// 	Returns the Mumble API.
		///----------------------------------------------------------------------------------------------------
		MumbleReader& Mumble();

		private:
		/* Dependencies */
		CDataLinkApi&          _DataLink;
		Host::EventApi&        _EventApi;
		CLogApi&               _Logger;
		Platform::RawInputApi& _RawInputApi;
		HWND                   _GameWindow;
		Network::CHttpClient&  _ArenaNetAssetCDN;

		std::filesystem::path _GameBindsPath;

		/* Services */
		std::unique_ptr<ArcdpsApi>        _Arcdps   { nullptr };
		std::unique_ptr<BuildInfoService> _BuildInfo{ nullptr };
		std::unique_ptr<GameBindsApi>     _GameBinds{ nullptr };
		std::unique_ptr<MumbleReader>     _Mumble   { nullptr };
	};
}
