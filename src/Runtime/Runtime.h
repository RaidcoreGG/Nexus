///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Runtime.h
/// Description  :  Nexus runtime implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <memory>

#include "Core/DataLink/DlApi.h"
#include "Core/Functions/FnRegistry.h"
#include "Core/Logging/LogApi.h"
#include "Core/Settings/SettingsMgr.h"
#include "Core/Versioning/Version.h"
#include "Graphics/GrMetrics.h"
#include "Graphics/GrWindow.h"
#include "Graphics/Textures/TxLoader.h"
#include "GW2/ArcDPS/ArcApi.h"
#include "GW2/BuildInfo/BuildInfoService.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Mumble/MblReader.h"
#include "Host/Config/CfgManager.h"
#include "Host/Events/EvtApi.h"
#include "Host/Library/LibManager.h"
#include "Host/Loader/Loader.h"
#include "Inputs/InputBinds/IbApi.h"
#include "Network/NetContext.h"
#include "Platform/PlContext.h"
#include "Proxy/PxyEnum.h"
#include "UI/UiContext.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus
{
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

		///----------------------------------------------------------------------------------------------------
		/// Logger:
		/// 	Returns the logger instance.
		///----------------------------------------------------------------------------------------------------
		Core::LogApi& Logger();

		///----------------------------------------------------------------------------------------------------
		/// DataLink:
		/// 	Returns the data link API instance.
		///----------------------------------------------------------------------------------------------------
		Core::DataLinkApi& DataLink();

		///----------------------------------------------------------------------------------------------------
		/// FunctionRegistry:
		/// 	Returns the function registry instance.
		///----------------------------------------------------------------------------------------------------
		Core::FuncRegistry& FunctionRegistry();

		///----------------------------------------------------------------------------------------------------
		/// Settings:
		/// 	Returns the settings instance.
		///----------------------------------------------------------------------------------------------------
		Core::SettingsMgr& Settings();

		Network::Context& Network();

		Platform::Context& Platform();

		///----------------------------------------------------------------------------------------------------
		/// Config:
		/// 	Returns the config instance.
		///----------------------------------------------------------------------------------------------------
		Host::ConfigMgr& Config();

		///----------------------------------------------------------------------------------------------------
		/// Loader:
		/// 	Returns the loader instance.
		///----------------------------------------------------------------------------------------------------
		Host::Loader& Loader();

		///----------------------------------------------------------------------------------------------------
		/// Library:
		/// 	Returns the library manager instance.
		///----------------------------------------------------------------------------------------------------
		Host::LibraryMgr& Library();

		///----------------------------------------------------------------------------------------------------
		/// Events:
		/// 	Returns the event API instance.
		///----------------------------------------------------------------------------------------------------
		Host::EventApi& Events();

		///----------------------------------------------------------------------------------------------------
		/// TextureLoader:
		/// 	Returns the texture loader instance.
		///----------------------------------------------------------------------------------------------------
		Graphics::TextureLoader& TextureLoader();

		///----------------------------------------------------------------------------------------------------
		/// GrMetrics:
		/// 	Returns the metrics.
		///----------------------------------------------------------------------------------------------------
		Graphics::Metrics_t& GrMetrics();

		///----------------------------------------------------------------------------------------------------
		/// GrWindow:
		/// 	Returns the window data.
		///----------------------------------------------------------------------------------------------------
		Graphics::Window_t& GrWindow();

		///----------------------------------------------------------------------------------------------------
		/// Arcdps:
		/// 	Returns the ArcDPS API.
		///----------------------------------------------------------------------------------------------------
		GW2::ArcdpsApi& Arcdps();

		///----------------------------------------------------------------------------------------------------
		/// Build:
		/// 	Returns the current game build information.
		///----------------------------------------------------------------------------------------------------
		GW2::BuildInfoService& BuildInfo();

		///----------------------------------------------------------------------------------------------------
		/// GameBinds:
		/// 	Returns the GameBinds API.
		///----------------------------------------------------------------------------------------------------
		GW2::GameBindsApi& GameBinds();

		///----------------------------------------------------------------------------------------------------
		/// Mumble:
		/// 	Returns the Mumble API.
		///----------------------------------------------------------------------------------------------------
		GW2::MumbleReader& Mumble();

		Input::CInputBindApi* InputBinds();

		GUI::Context& UI();

		private:
		Runtime();
		~Runtime();

		std::unique_ptr<Network::Context>  _NetworkContext{ nullptr };
		std::unique_ptr<Platform::Context> _PlatformContext{ nullptr };
		std::unique_ptr<GUI::Context>      _UiContext{ nullptr };
	};
}
