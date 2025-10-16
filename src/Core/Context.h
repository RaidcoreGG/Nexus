///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.h
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>
#include <mutex>

#include "Core/Addons/Config/CfgManager.h"
#include "Core/Addons/Library/LibManager.h"
#include "Core/Preferences/PrefContext.h"
#include "Core/Updater/SelfUpdater.h"
#include "Core/Versioning/MajorMinorBuildRevision.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "Engine/Renderer/RdrContext.h"
#include "Engine/Textures/TxLoader.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/UiContext.h"

class CContext
{
	public:
	static CContext* GetContext();

	CContext(CContext const&)       = delete;
	void operator=(CContext const&) = delete;

	MajorMinorBuildRevision_t const& GetVersion();

	const char* GetBuild();

	void SetModule(HMODULE aModule);

	HMODULE GetModule();

	DWORD GetModuleSize();

	RenderContext_t* GetRendererCtx();

	CLogApi* GetLogger();

	CTextureLoader* GetTextureService();

	CDataLinkApi* GetDataLink();

	CEventApi* GetEventApi();

	CLoader* GetLoader();

	CLibraryMgr* GetAddonLibrary();

	CRawInputApi* GetRawInputApi();

	CInputBindApi* GetInputBindApi();

	CGameBindsApi* GetGameBindsApi();

	CUiContext* GetUIContext();

	CSettings* GetSettingsCtx();

	CMumbleReader* GetMumbleReader();

	CHttpClient* GetHttpClient(std::string aURL);

	CSelfUpdater* GetSelfUpdater();

	CArcApi* GetArcApi();

	CConfigMgr* GetCfgMgr();

	private:
	CContext() = default;
	~CContext();

	HMODULE                             Module     = nullptr;
	DWORD                               ModuleSize = 0;

	std::mutex                          HttpClientMutex;
	std::map<std::string, CHttpClient*> HttpClients;
};

#endif
