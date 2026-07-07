///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Runtime.h
/// Description  :  Nexus runtime implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <windows.h>

#include "Core/Addons/Config/CfgManager.h"
#include "Core/Addons/Library/LibManager.h"
#include "Core/Preferences/PrefContext.h"
#include "Core/Updater/SelfUpdater.h"
#include "Core/Versioning/Version.h"
#include "Engine/CrashHandler/CrashHandler.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "Proxy/PxyEnum.h"
#include "UI/Renderer/RdrContext.h"
#include "UI/Textures/TxLoader.h"
#include "UI/UiContext.h"

constexpr const char* CH_CORE = "Core";

class Runtime
{
	public:
	static Runtime& Get();

	Runtime(Runtime const&)       = delete;
	void operator=(Runtime const&) = delete;

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Initializes the addon engine.
	///----------------------------------------------------------------------------------------------------
	void Initialize(EProxyFunction aEntryFunction);

	///----------------------------------------------------------------------------------------------------
	/// Shutdown:
	/// 	Shuts down the addon engine.
	///----------------------------------------------------------------------------------------------------
	void Shutdown(unsigned int aReason);

	Version_t const& GetVersion();

	const char* GetBuild();

	HMODULE GetModule();

	DWORD GetModuleSize();

	CCrashHandler* GetCrashHandler();

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
	Runtime();
	~Runtime();

	HMODULE          Module{ nullptr };
	DWORD            ModuleSize{ 0 };

	std::mutex                          HttpClientMutex{};
	std::map<std::string, CHttpClient*> HttpClients{};

};
