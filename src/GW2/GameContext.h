///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GameContext.h
/// Description  :  Guild Wars 2 game context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <utility>

#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/BuildInfo/BuildInfoService.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/Renderer/RdrContext.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2 Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2
{
	///----------------------------------------------------------------------------------------------------
	/// GameContext Class
	///----------------------------------------------------------------------------------------------------
	class GameContext
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		GameContext(
			CDataLinkApi&         aDataLink,
			CEventApi&            aEventApi,
			CLogApi&              aLogger,
			CRawInputApi&         aRawInputApi,
			RenderContext_t&      aRendererCtx,
			CHttpClient&          aArenaNetAssetCDN,
			std::filesystem::path aGameBindsPath
		)
			: _DataLink(aDataLink)
			, _EventApi(aEventApi)
			, _Logger(aLogger)
			, _RawInputApi(aRawInputApi)
			, _RendererCtx(aRendererCtx)
			, _ArenaNetAssetCDN(aArenaNetAssetCDN)
			, _GameBindsPath(std::move(aGameBindsPath))
		{}

		///----------------------------------------------------------------------------------------------------
		/// Shutdown:
		/// 	Shuts down the game context.
		///----------------------------------------------------------------------------------------------------
		void Shutdown();

		///----------------------------------------------------------------------------------------------------
		/// Arcdps:
		/// 	Returns the ArcDPS API.
		///----------------------------------------------------------------------------------------------------
		CArcApi& Arcdps();

		///----------------------------------------------------------------------------------------------------
		/// Build:
		/// 	Returns the current game build information.
		///----------------------------------------------------------------------------------------------------
		BuildInfoService& BuildInfo();

		///----------------------------------------------------------------------------------------------------
		/// GameBinds:
		/// 	Returns the GameBinds API.
		///----------------------------------------------------------------------------------------------------
		CGameBindsApi& GameBinds();

		///----------------------------------------------------------------------------------------------------
		/// Mumble:
		/// 	Returns the Mumble API.
		///----------------------------------------------------------------------------------------------------
		CMumbleReader& Mumble();

		private:
		/* Dependencies */
		CDataLinkApi&    _DataLink;
		CEventApi&       _EventApi;
		CLogApi&         _Logger;
		CRawInputApi&    _RawInputApi;
		RenderContext_t& _RendererCtx;
		CHttpClient&     _ArenaNetAssetCDN;

		std::filesystem::path _GameBindsPath;

		/* Services */
		std::unique_ptr<CArcApi>          _Arcdps   { nullptr };
		std::unique_ptr<BuildInfoService> _BuildInfo{ nullptr };
		std::unique_ptr<CGameBindsApi>    _GameBinds{ nullptr };
		std::unique_ptr<CMumbleReader>    _Mumble   { nullptr };
	};
}
