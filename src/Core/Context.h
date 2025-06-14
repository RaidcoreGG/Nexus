///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.h
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef CONTEXT_H
#define CONTEXT_H

#include "Core/Preferences/PrefContext.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/AddonVersion.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "Engine/Renderer/RdrContext.h"
#include "Engine/Textures/TxLoader.h"
#include "Engine/Updater/Updater.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/UiContext.h"

class CContext
{
	public:
	static CContext* GetContext();

	CContext(CContext const&)       = delete;
	void operator=(CContext const&) = delete;

	AddonVersion_t const& GetVersion();

	const char* GetBuild();

	void SetModule(HMODULE aModule);

	HMODULE GetModule();

	DWORD GetModuleSize();

	RenderContext_t* GetRendererCtx();

	CLogApi* GetLogger();

	CUpdater* GetUpdater();

	CTextureLoader* GetTextureService();

	CDataLinkApi* GetDataLink();

	CEventApi* GetEventApi();

	CRawInputApi* GetRawInputApi();

	CInputBindApi* GetInputBindApi();

	CGameBindsApi* GetGameBindsApi();

	CUiContext* GetUIContext();

	CSettings* GetSettingsCtx();

	CMumbleReader* GetMumbleReader();

	CHttpClient* GetRaidcoreApi();

	CHttpClient* GetGitHubApi();

	private:
	CContext() = default;

	HMODULE Module;
	DWORD   ModuleSize;
};

#endif
