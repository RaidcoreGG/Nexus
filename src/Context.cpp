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
#include "Index/Index.h"
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

CLogApi* CContext::GetLogger()
{
	static CLogApi s_Logger = CLogApi();
	return &s_Logger;
}

CLocalization* CContext::GetLocalization()
{
	static CLocalization s_LocalizationApi = CLocalization(
		this->GetLogger(),
		this->GetEventApi()
	);
	return &s_LocalizationApi;
}

CUpdater* CContext::GetUpdater()
{
	static CUpdater s_Upater = CUpdater(
		this->GetLogger()
	);
	return &s_Upater;
}

CTextureLoader* CContext::GetTextureService()
{
	static CTextureLoader s_TextureApi = CTextureLoader(
		this->GetLogger()
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
	static CRawInputApi s_RawInputApi = CRawInputApi();
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
		this->GetEventApi(),
		this->GetLocalization()
	);
	return &s_GameBindsApi;
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
