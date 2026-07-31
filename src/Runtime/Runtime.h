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
#include "GW2/Gw2Context.h"
#include "Host/HoContext.h"
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

		Host::Context& Host();

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

		GW2::Context& Game();

		Input::CInputBindApi* InputBinds();

		GUI::Context& UI();

		private:
		Runtime();
		~Runtime();

		std::unique_ptr<Network::Context>  _NetworkContext{ nullptr };
		std::unique_ptr<Platform::Context> _PlatformContext{ nullptr };
		std::unique_ptr<Host::Context>     _HostContext{ nullptr };
		std::unique_ptr<GW2::Context>      _GameContext{ nullptr };
		std::unique_ptr<GUI::Context>      _UiContext{ nullptr };
	};
}
