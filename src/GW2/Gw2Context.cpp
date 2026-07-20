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
#include "thirdparty/Clockwork/Clockwork.h"
#include "Inputs/GameBinds/GbApi.h"
#include "Mumble/MblReader.h"
#include "Multibox/Multibox.h"
#include "Index/IdxEnum.h"
#include "Index/Index.h"

namespace Raidcore::Nexus::GW2
{
	Context::Context(
		Core::DataLinkApi&          aDataLink,
		Host::EventApi&        aEventApi,
		Core::LogApi&               aLogger,
		Platform::RawInputApi& aRawInputApi,
		HWND                   aGameWindow,
		Network::CHttpClient&  aArenaNetAssetCDN
	)
		: _DataLink(aDataLink)
		, _EventApi(aEventApi)
		, _Logger(aLogger)
		, _RawInputApi(aRawInputApi)
		, _GameWindow(aGameWindow)
		, _ArenaNetAssetCDN(aArenaNetAssetCDN)
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
			this->_GameWindow,
			Index(EPath::GameBinds)
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
