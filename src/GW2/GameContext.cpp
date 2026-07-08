///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GameContext.cpp
/// Description  :  Guild Wars 2 game context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "GameContext.h"

#include <memory>

#include "ArcDPS/ArcApi.h"
#include "BuildInfo/BuildInfoService.h"
#include "Inputs/GameBinds/GbApi.h"
#include "Mumble/MblReader.h"

namespace Raidcore::Nexus::GW2
{
	void GameContext::Shutdown()
	{
		this->_Mumble.reset();
		this->_GameBinds.reset();
		this->_BuildInfo.reset();
		this->_Arcdps.reset();
	}

	CArcApi& GameContext::Arcdps()
	{
		if (!this->_Arcdps)
		{
			this->_Arcdps = std::make_unique<CArcApi>();
		}

		return *this->_Arcdps;
	}

	BuildInfoService& GameContext::BuildInfo()
	{
		if (!this->_BuildInfo)
		{
			this->_BuildInfo = std::make_unique<BuildInfoService>(
				this->_ArenaNetAssetCDN,
				this->_Logger
			);
		}

		return *this->_BuildInfo;
	}

	CGameBindsApi& GameContext::GameBinds()
	{
		if (!this->_GameBinds)
		{
			this->_GameBinds = std::make_unique<CGameBindsApi>(
				this->_RawInputApi,
				this->_Logger,
				this->_EventApi,
				this->_RendererCtx,
				this->_GameBindsPath
			);
		}

		return *this->_GameBinds;
	}

	CMumbleReader& GameContext::Mumble()
	{
		if (!this->_Mumble)
		{
			this->_Mumble = std::make_unique<CMumbleReader>(
				this->_DataLink,
				this->_EventApi,
				this->_Logger
			);
		}

		return *this->_Mumble;
	}
}
