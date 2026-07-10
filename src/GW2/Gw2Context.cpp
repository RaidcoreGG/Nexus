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
#include "Engine/Clockwork/Clockwork.h"
#include "Inputs/GameBinds/GbApi.h"
#include "Mumble/MblReader.h"
#include "Multibox/Multibox.h"

namespace Raidcore::Nexus::GW2
{
	Context::Context(
		CDataLinkApi&           aDataLink,
		Host::CEventApi&        aEventApi,
		CLogApi&                aLogger,
		Platform::RawInputApi& aRawInputApi,
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
		this->_Arcdps = std::make_unique<ArcdpsApi>();
		this->_BuildInfo = std::make_unique<BuildInfoService>(
			this->_ArenaNetAssetCDN,
			this->_Logger
		);
		this->_GameBinds = std::make_unique<GameBindsApi>(
			this->_RawInputApi,
			this->_Logger,
			this->_EventApi,
			this->_RendererCtx,
			this->_GameBindsPath
		);
		this->_Mumble = std::make_unique<MumbleReader>(
			this->_DataLink,
			this->_EventApi,
			this->_Logger
		);
	}

	void Context::Initialize()
	{
		/* Prefetch game build. */
		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Immediate, [this](Clockwork::CancellationToken aToken)
		{
			this->BuildInfo().Build();
		});

		/* Set up multiboxing. */
		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Low, [this](Clockwork::CancellationToken aToken)
		{
			Multibox::KillMutex();
			this->_Logger.Info(LOG_CHANNEL, "Multibox State: %d", Multibox::GetState());
		});
	}

	void Context::Shutdown()
	{
		this->_Mumble.reset();
		this->_GameBinds.reset();
		this->_BuildInfo.reset();
		this->_Arcdps.reset();
	}

	ArcdpsApi& Context::Arcdps()
	{
		return *this->_Arcdps;
	}

	BuildInfoService& Context::BuildInfo()
	{
		return *this->_BuildInfo;
	}

	GameBindsApi& Context::GameBinds()
	{
		return *this->_GameBinds;
	}

	MumbleReader& Context::Mumble()
	{
		return *this->_Mumble;
	}
}
