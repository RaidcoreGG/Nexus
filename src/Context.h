///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.h
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Events/EventHandler.h"
#include "Inputs/GameBinds/GameBindsHandler.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Inputs/RawInput/RawInputApi.h"
#include "Loader/AddonVersion.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Localization/Localization.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Settings/Settings.h"
#include "Services/Textures/TextureLoader.h"
#include "Services/Updater/Updater.h"
#include "UI/UiContext.h"

#ifndef CONTEXT_H
#define CONTEXT_H

class CContext
{
	public:
	static CContext* GetContext();

	CContext(CContext const&)       = delete;
	void operator=(CContext const&) = delete;

	AddonVersion const& GetVersion();

	const char* GetBuild();

	void SetModule(HMODULE aModule);
	HMODULE GetModule();

	void SetLogger(CLogHandler* aLogger);
	CLogHandler* GetLogger();

	void SetLocalization(CLocalization* aLocalization);
	CLocalization* GetLocalization();

	void SetUpdater(CUpdater* aUpdater);
	CUpdater* GetUpdater();

	void SetTextureService(CTextureLoader* aTextureService);
	CTextureLoader* GetTextureService();

	void SetDataLink(CDataLink* aDataLink);
	CDataLink* GetDataLink();

	void SetEventApi(CEventApi* aEventApi);
	CEventApi* GetEventApi();

	void SetRawInputApi(CRawInputApi* aRawInputApi);
	CRawInputApi* GetRawInputApi();

	void SetInputBindApi(CInputBindApi* aInputBindApi);
	CInputBindApi* GetInputBindApi();

	void SetGameBindsApi(CGameBindsApi* aGameBindsApi);
	CGameBindsApi* GetGameBindsApi();

	void SetUIContext(CUiContext* aUiContext);
	CUiContext* GetUIContext();

	void SetSettingsCtx(CSettings* aSettingsCtx);
	CSettings* GetSettingsCtx();

	private:
	CContext() = default;

	HMODULE         Module;

	CLogHandler*    Logger;
	CLocalization*  Language;
	CUpdater*       UpdateService;
	CTextureLoader* TextureService;
	CDataLink*      DataLinkService;
	CEventApi*      EventApi;
	CRawInputApi*   RawInputApi;
	CInputBindApi*  InputBindApi;
	CGameBindsApi*  GameBindsApi;
	CUiContext*     UIContext;
	CSettings*      Settings;
};

#endif
