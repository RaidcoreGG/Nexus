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

#include "Core/CoContext.h"
#include "Core/Versioning/Version.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Graphics/GrContext.h"
#include "GW2/Gw2Context.h"
#include "Host/HoContext.h"
#include "Network/NetContext.h"
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

		Core::Context& Core();

		Network::Context& Network();

		Platform::Context& Platform();

		Host::Context& Host();

		Graphics::Context& Graphics();

		GW2::Context& Game();

		CInputBindApi* InputBinds();

		GUI::Context& UI();

		private:
		Runtime();
		~Runtime();

		std::unique_ptr<Core::Context>     _CoreContext    { nullptr };
		std::unique_ptr<Network::Context>  _NetworkContext { nullptr };
		std::unique_ptr<Platform::Context> _PlatformContext{ nullptr };
		std::unique_ptr<Host::Context>     _HostContext    { nullptr };
		std::unique_ptr<Graphics::Context> _GraphicsContext{ nullptr };
		std::unique_ptr<GW2::Context>      _GameContext    { nullptr };
		std::unique_ptr<GUI::Context>      _UiContext      { nullptr };
	};
}
