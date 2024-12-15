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
#include "Index.h"
#include "Version.h"

CContext* CContext::GetContext()
{
	static CContext s_Context;
	return &s_Context;
}

AddonVersion const& CContext::GetVersion()
{
	static AddonVersion version =
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

CDataLink* CContext::GetDataLink()
{
	static CDataLink s_DataLinkApi = CDataLink(
		this->GetLogger()
	);
	return &s_DataLinkApi;
}

CEventApi* CContext::GetEventApi()
{
	static CEventApi s_EventApi = CEventApi();
	return &s_EventApi;
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

CInputBindApi* CContext::GetInputBindApi()
{
	static CInputBindApi s_InputBindApi = CInputBindApi(
		this->GetEventApi(),
		this->GetLogger()
	);
	return &s_InputBindApi;
}

CLoader* CContext::GetLoader()
{
	static CLoader s_Loader = CLoader();
	return &s_Loader;
}

CLocalization* CContext::GetLocalization()
{
	static CLocalization s_LocalizationApi = CLocalization(
		this->GetLogger(),
		this->GetEventApi()
	);
	return &s_LocalizationApi;
}

CLogHandler* CContext::GetLogger()
{
	static CLogHandler s_Logger = CLogHandler();
	return &s_Logger;
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

CRawInputApi* CContext::GetRawInputApi()
{
	static CRawInputApi s_RawInputApi = CRawInputApi();
	return &s_RawInputApi;
}

CSettings* CContext::GetSettingsCtx()
{
	static CSettings s_SettingsApi = CSettings(
		Index::F_SETTINGS,
		this->GetLogger()
	);
	return &s_SettingsApi;
}

CTextureLoader* CContext::GetTextureService()
{
	static CTextureLoader s_TextureApi = CTextureLoader(
		this->GetLogger()
	);
	return &s_TextureApi;
}

CUiContext* CContext::GetUIContext()
{
	static CUiContext s_UiContext = CUiContext(
		this->GetLogger(),
		this->GetLocalization(),
		this->GetTextureService(),
		this->GetDataLink(),
		this->GetInputBindApi(),
		this->GetEventApi()
	);
	return &s_UiContext;
}

CUpdater* CContext::GetUpdater()
{
	static CUpdater s_Upater = CUpdater(
		this->GetLogger()
	);
	return &s_Upater;
}
