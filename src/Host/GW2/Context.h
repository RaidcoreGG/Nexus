///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.h
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <mutex>

#include "Host/GW2/Addons/Config/CfgManager.h"
#include "Host/GW2/Addons/Library/LibManager.h"
#include "Core/Platform/Windows/Exe/ExeCrashHandler.h"
#include "Host/GW2/Updater/SelfUpdater.h"
#include "Core/Versioning/Version.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/Loader.h"
#include "Core/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "Host/GW2/ArcDPS/ArcApi.h"
#include "Host/GW2/Events/EvtRouter.h"
#include "Host/GW2/Hooks/HkGameWindow.h"
#include "Host/GW2/Inputs/GameBinds/GbApi.h"
#include "Host/GW2/Inputs/MouseResetFix.h"
#include "Host/GW2/Mumble/MblReader.h"
#include "UI/Renderer/RdrContext.h"
#include "UI/Textures/TxLoader.h"
#include "UI/UiContext.h"

namespace Raidcore::GW2
{
	class CContext
	{
		public:
		static CContext* GetContext();

		CContext(CContext const&)       = delete;
		void operator=(CContext const&) = delete;

		Core::Version_t const& GetVersion();

		const char* GetBuild();

		void SetModule(HMODULE aModule);

		HMODULE GetModule();

		DWORD GetModuleSize();

		Core::Windows::CCrashHandler* GetCrashHandler();

		UI::RenderContext_t* GetRendererCtx();

		UI::CTextureLoader* GetTextureService();

		Engine::CDataLinkApi* GetDataLink();

		Engine::CEventApi* GetEventApi();

		GW2::CEventRouter* GetEventRouter();

		Engine::CLoader* GetLoader();

		GW2::CLibraryMgr* GetAddonLibrary();

		Engine::Input::CRawInputApi* GetRawInputApi();

		Engine::Input::CInputBindApi* GetInputBindApi();

		GW2::CGameBindsApi* GetGameBindsApi();

		GW2::CMouseResetFix* GetMouseResetFix();

		GW2::Hooks::CGameWindow* GetGameWindow();

		UI::CUiContext* GetUIContext();

		GW2::CMumbleReader* GetMumbleReader();

		Engine::Networking::CHttpClient* GetHttpClient(std::string aURL, bool aDisableCache = false);

		///----------------------------------------------------------------------------------------------------
		/// StopHttpClients:
		/// 	Stops every HTTP client, including ones created afterwards, so no request blocks shutdown.
		///----------------------------------------------------------------------------------------------------
		void StopHttpClients();

		GW2::CSelfUpdater* GetSelfUpdater();

		GW2::ArcDPS::CArcApi* GetArcApi();

		GW2::CConfigMgr* GetCfgMgr();

		private:
		CContext() = default;
		~CContext();

		HMODULE                             Module     = nullptr;
		DWORD                               ModuleSize = 0;

		std::mutex                          HttpClientMutex;
		std::map<std::string, Engine::Networking::CHttpClient*> HttpClients;
		bool                                IsHttpStopped = false;
	};
}
