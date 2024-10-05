///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.cpp
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Context.h"

#include "Branch.h"
#include "Version.h"

CContext* CContext::GetContext()
{
	static CContext context;
	return &context;
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
}
HMODULE CContext::GetModule()
{
	return this->Module;
}

void CContext::SetLogger(CLogHandler* aLogger)
{
	assert(!this->Logger);
	this->Logger = aLogger;
}
CLogHandler* CContext::GetLogger()
{
	return this->Logger;
}

void CContext::SetLocalization(CLocalization* aLocalization)
{
	assert(!this->Language);
	this->Language = aLocalization;
}
CLocalization* CContext::GetLocalization()
{
	return this->Language;
}

void CContext::SetUpdater(CUpdater* aUpdater)
{
	assert(!this->UpdateService);
	this->UpdateService = aUpdater;
}
CUpdater* CContext::GetUpdater()
{
	return this->UpdateService;
}

void CContext::SetTextureService(CTextureLoader* aTextureService)
{
	assert(!this->TextureService);
	this->TextureService = aTextureService;
}
CTextureLoader* CContext::GetTextureService()
{
	return this->TextureService;
}

void CContext::SetDataLink(CDataLink* aDataLink)
{
	assert(!this->DataLinkService);
	this->DataLinkService = aDataLink;
}
CDataLink* CContext::GetDataLink()
{
	return this->DataLinkService;
}

void CContext::SetEventApi(CEventApi* aEventApi)
{
	assert(!this->EventApi);
	this->EventApi = aEventApi;
}
CEventApi* CContext::GetEventApi()
{
	return this->EventApi;
}

void CContext::SetRawInputApi(CRawInputApi* aRawInputApi)
{
	assert(!this->RawInputApi);
	this->RawInputApi = aRawInputApi;
}
CRawInputApi* CContext::GetRawInputApi()
{
	return this->RawInputApi;
}

void CContext::SetInputBindApi(CInputBindApi* aInputBindApi)
{
	assert(!this->InputBindApi);
	this->InputBindApi = aInputBindApi;
}
CInputBindApi* CContext::GetInputBindApi()
{
	return this->InputBindApi;
}

void CContext::SetGameBindsApi(CGameBindsApi* aGameBindsApi)
{
	assert(!this->GameBindsApi);
	this->GameBindsApi = aGameBindsApi;
}
CGameBindsApi* CContext::GetGameBindsApi()
{
	return this->GameBindsApi;
}

void CContext::SetUIContext(CUiContext* aUiContext)
{
	assert(!this->UIContext);
	this->UIContext = aUiContext;
}
CUiContext* CContext::GetUIContext()
{
	return this->UIContext;
}

void CContext::SetSettingsCtx(CSettings* aSettingsCtx)
{
	assert(!this->Settings);
	this->Settings = aSettingsCtx;
}
CSettings* CContext::GetSettingsCtx()
{
	return this->Settings;
}
