///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GameContext.h
/// Description  :  Guild Wars 2 game context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>

#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"

class GameContext
{
	public:
	GameContext(
		CDataLinkApi&         aDataLink,
		CEventApi&            aEventApi,
		CLogApi&              aLogger,
		CRawInputApi&         aRawInputApi,
		RenderContext_t&      aRendererCtx,
		std::filesystem::path aGameBindsPath
	)
		: _DataLink(aDataLink)
		, _EventApi(aEventApi)
		, _Logger(aLogger)
		, _RawInputApi(aRawInputApi)
		, _RendererCtx(aRendererCtx)
		, _GameBindsPath(std::move(aGameBindsPath))
	{}

	void Shutdown();

	CArcApi&       Arcdps();
	CGameBindsApi& GameBinds();
	CMumbleReader& Mumble();

	private:
	/* Dependencies */
	CDataLinkApi&    _DataLink;
	CEventApi&       _EventApi;
	CLogApi&         _Logger;
	CRawInputApi&    _RawInputApi;
	RenderContext_t& _RendererCtx;

	std::filesystem::path _GameBindsPath;

	/* Services */
	std::unique_ptr<CArcApi>       _Arcdps   { nullptr };
	std::unique_ptr<CGameBindsApi> _GameBinds{ nullptr };
	std::unique_ptr<CMumbleReader> _Mumble   { nullptr };
};
