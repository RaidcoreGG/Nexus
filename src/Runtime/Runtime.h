///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Runtime.h
/// Description  :  Nexus runtime implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <windows.h>

#include "Core/Preferences/PrefContext.h"
#include "Core/Updater/SelfUpdater.h"
#include "Core/Versioning/Version.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "Graphics/GrContext.h"
#include "GW2/Gw2Context.h"
#include "Host/HoContext.h"
#include "Platform/PlContext.h"
#include "Proxy/PxyEnum.h"
#include "UI/UiContext.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus
{
	constexpr const char* CH_CORE = "Core";

	class Runtime
	{
		public:
		static Runtime& Get();

		Runtime(Runtime const&) = delete;
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

		Platform::Context& Platform();

		Host::Context& Host();

		Graphics::Context& Graphics();

		GW2::Context& Game();

		CLogApi* GetLogger();

		CDataLinkApi* GetDataLink();

		CInputBindApi* GetInputBindApi();

		CUiContext* GetUIContext();

		CSettings* GetSettingsCtx();

		CHttpClient* GetHttpClient(std::string aURL, bool aDisableCache = false);

		CSelfUpdater* GetSelfUpdater();

		private:
		Runtime();
		~Runtime();

		std::unique_ptr<Platform::Context> _PlatformContext{ nullptr };
		std::unique_ptr<Host::Context>     _HostContext    { nullptr };
		std::unique_ptr<Graphics::Context> _GraphicsContext{ nullptr };
		std::unique_ptr<GW2::Context>      _GameContext    { nullptr };

		std::mutex                          HttpClientMutex{};
		std::map<std::string, CHttpClient*> HttpClients{};
	};
}
