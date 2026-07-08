///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Gw2Context.cpp
/// Description  :  Guild Wars 2 game context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Gw2Context.h"

#include <memory>

#include "ArcDPS/ArcApi.h"
#include "BuildInfo/BuildInfoService.h"
#include "Inputs/GameBinds/GbApi.h"
#include "Mumble/MblReader.h"

namespace Raidcore::Nexus::GW2
{
	Context::Context(
		CDataLinkApi&           aDataLink,
		CEventApi&              aEventApi,
		CLogApi&                aLogger,
		Platform::CRawInputApi& aRawInputApi,
		RenderContext_t&        aRendererCtx,
		CHttpClient&            aArenaNetAssetCDN,
		std::filesystem::path   aGameBindsPath
	)
		: _DataLink(aDataLink)
		, _EventApi(aEventApi)
		, _Logger(aLogger)
		, _RawInputApi(aRawInputApi)
		, _RendererCtx(aRendererCtx)
		, _ArenaNetAssetCDN(aArenaNetAssetCDN)
		, _GameBindsPath(std::move(aGameBindsPath))
	{
		this->_Arcdps = std::make_unique<CArcApi>();
		this->_BuildInfo = std::make_unique<BuildInfoService>(
			this->_ArenaNetAssetCDN,
			this->_Logger
		);
		this->_GameBinds = std::make_unique<CGameBindsApi>(
			this->_RawInputApi,
			this->_Logger,
			this->_EventApi,
			this->_RendererCtx,
			this->_GameBindsPath
		);
		this->_Mumble = std::make_unique<CMumbleReader>(
			this->_DataLink,
			this->_EventApi,
			this->_Logger
		);
	}

	void Context::Shutdown()
	{
		this->_Mumble.reset();
		this->_GameBinds.reset();
		this->_BuildInfo.reset();
		this->_Arcdps.reset();
	}

	CArcApi& Context::Arcdps()
	{
		return *this->_Arcdps;
	}

	BuildInfoService& Context::BuildInfo()
	{
		return *this->_BuildInfo;
	}

	CGameBindsApi& Context::GameBinds()
	{
		return *this->_GameBinds;
	}

	CMumbleReader& Context::Mumble()
	{
		return *this->_Mumble;
	}
}
