///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.cpp
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Context.h"

#include <Psapi.h>

#include "Branch.h"
#include "Engine/Index/Index.h"
#include "Version.h"

CContext* CContext::GetContext()
{
	static CContext s_Context;
	return &s_Context;
}

AddonVersion_t const& CContext::GetVersion()
{
	static AddonVersion_t version =
	{
		V_MAJOR,
		V_MINOR,
		V_BUILD,
		V_REVISION
	};
	return version;
}

const char* CContext::GetBuild()
{
#ifdef _DEBUG
	return "debug/" BRANCH_NAME;
#else
	return "release/" BRANCH_NAME;
#endif
}

void CContext::SetModule(HMODULE aModule)
{
	assert(!this->Module);
	this->Module = aModule;

	MODULEINFO moduleInfo{};
	GetModuleInformation(GetCurrentProcess(), this->Module, &moduleInfo, sizeof(moduleInfo));
	this->ModuleSize = moduleInfo.SizeOfImage;
}

HMODULE CContext::GetModule()
{
	return this->Module;
}

DWORD CContext::GetModuleSize()
{
	return this->ModuleSize;
}

RenderContext_t* CContext::GetRendererCtx()
{
	static RenderContext_t s_RendererCtx = RenderContext_t();
	return &s_RendererCtx;
}

CLogApi* CContext::GetLogger()
{
	static CLogApi s_Logger = CLogApi();
	return &s_Logger;
}

CUpdater* CContext::GetUpdater()
{
	static CUpdater s_Updater = CUpdater(
		this->GetLogger()
	);
	return &s_Updater;
}

CTextureLoader* CContext::GetTextureService()
{
	static CTextureLoader s_TextureApi = CTextureLoader(
		this->GetLogger(),
		this->GetRendererCtx()
	);
	return &s_TextureApi;
}

CDataLinkApi* CContext::GetDataLink()
{
	static CDataLinkApi s_DataLinkApi = CDataLinkApi(
		this->GetLogger()
	);
	return &s_DataLinkApi;
}

CEventApi* CContext::GetEventApi()
{
	static CEventApi s_EventApi = CEventApi();
	return &s_EventApi;
}

CRawInputApi* CContext::GetRawInputApi()
{
	static CRawInputApi s_RawInputApi = CRawInputApi(
		this->GetRendererCtx()
	);
	return &s_RawInputApi;
}

CInputBindApi* CContext::GetInputBindApi()
{
	static CInputBindApi s_InputBindApi = CInputBindApi(
		this->GetEventApi(),
		this->GetLogger()
	);
	return &s_InputBindApi;
}

CGameBindsApi* CContext::GetGameBindsApi()
{
	static CGameBindsApi s_GameBindsApi = CGameBindsApi(
		this->GetRawInputApi(),
		this->GetLogger(),
		this->GetEventApi()
	);
	return &s_GameBindsApi;
}

CUiContext* CContext::GetUIContext()
{
	static CUiContext s_UiContext = CUiContext(
		this->GetRendererCtx(),
		this->GetLogger(),
		this->GetTextureService(),
		this->GetDataLink(),
		this->GetInputBindApi(),
		this->GetEventApi(),
		this->GetMumbleReader()
	);
	return &s_UiContext;
}

CSettings* CContext::GetSettingsCtx()
{
	static CSettings s_SettingsApi = CSettings(
		Index(EPath::Settings),
		this->GetLogger()
	);
	return &s_SettingsApi;
}

CMumbleReader* CContext::GetMumbleReader()
{
	static CMumbleReader s_MumbleReader = CMumbleReader(
		this->GetDataLink(),
		this->GetEventApi(),
		this->GetLogger()
	);
	return &s_MumbleReader;
}

CHttpClient* CContext::GetRaidcoreApi()
{
	static CHttpClient s_RaidcoreApiCli = CHttpClient(
		this->GetLogger(),
		"https://api.raidcore.gg",
		Index(EPath::DIR_APICACHE_RAIDCORE),
		5 * 60
	);
	return &s_RaidcoreApiCli;
}

CHttpClient* CContext::GetGitHubApi()
{
	static CHttpClient s_GitHubApiCli = CHttpClient(
		this->GetLogger(),
		"https://api.github.com",
		Index(EPath::DIR_APICACHE_GITHUB),
		30 * 60
	);
	return &s_GitHubApiCli;
}
