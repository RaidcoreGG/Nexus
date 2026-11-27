///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiContext.h
/// Description  :  Contains the core functionality of the User Interface.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <d3d11.h>
#include <map>
#include <vector>
#include <Windows.h>

#include "Engine/Inputs/RawInput/WndProcNotify.h"
#include "Host/GW2/Hooks/HkNotify.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/InputBinds/IbNotify.h"
#include "Core/Logging/LogChannel.h"
#include "Host/GW2/Mumble/MblNotify.h"
#include "Host/GW2/Mumble/MblReader.h"
#include "UI/Renderer/RdrContext.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/Services/Fonts/FontNotify.h"
#include "UI/Services/Localization/LoclApi.h"
#include "UI/Services/QoL/EscapeClosing.h"
#include "UI/Services/Scaling/Scaling.h"
#include "UI/Textures/TxLoader.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/EULA/LicenseAgreementModal.h"
#include "UI/Widgets/MainWindow/MainWindow.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"
#include "UiBinds.h"
#include "UiInput.h"
#include "UiRender.h"
#include "UiStyle.h"
#include "Util/Inputs.h"

namespace Raidcore::UI
{
	constexpr const char* KB_MENU            = "KB_MENU";
	constexpr const char* KB_ADDONS          = "KB_ADDONS";
	constexpr const char* KB_OPTIONS         = "KB_OPTIONS";
	constexpr const char* KB_LOG             = "KB_LOG";
	constexpr const char* KB_DEBUG           = "KB_DEBUG";
	constexpr const char* KB_MUMBLEOVERLAY   = "KB_MUMBLEOVERLAY";
	constexpr const char* KB_TOGGLEHIDEUI    = "KB_TOGGLEHIDEUI";

	///----------------------------------------------------------------------------------------------------
	/// CUiContext Class
	///----------------------------------------------------------------------------------------------------
	class CUiContext
		: public Core::ILogChannel
		, public CUiRender
		, public CUiBinds
		, public CUiStyle
		, public Engine::Input::IWndProcNotifyHandler
		, public GW2::Hooks::IDxgiPresentNotifyHandler
		, public GW2::Hooks::IDxgiResizeNotifyHandler
		, public GW2::IMumbleIdentityNotifyHandler
		, public Engine::Input::IInputBindUpdateNotifyHandler
		, public UI::IFontUpdateNotifyHandler
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// OnInputBindPressed:
		/// 	Receives input bind invocations.
		///----------------------------------------------------------------------------------------------------
		static void OnInputBindPressed(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		CUiContext(
			UI::RenderContext_t*      aRenderContext,
			UI::CTextureLoader*       aTextureService,
			Engine::CDataLinkApi*         aDataLink,
			Engine::Input::CInputBindApi* aInputBindApi,
			GW2::CMumbleReader*        aMumbleReader
		);

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~CUiContext();

		///----------------------------------------------------------------------------------------------------
		/// Initialize:
		/// 	Initializes the UI context.
		///----------------------------------------------------------------------------------------------------
		void Initialize();

		///----------------------------------------------------------------------------------------------------
		/// Shutdown:
		/// 	Shuts down the UI context.
		///----------------------------------------------------------------------------------------------------
		void Shutdown();

		///----------------------------------------------------------------------------------------------------
		/// Render:
		/// 	Renders the UI and executes callbacks.
		///----------------------------------------------------------------------------------------------------
		void Render();

		///----------------------------------------------------------------------------------------------------
		/// Invalidate:
		/// 	Calls all UI children's invalidate function, causing them to refresh.
		///----------------------------------------------------------------------------------------------------
		void Invalidate();

		///----------------------------------------------------------------------------------------------------
		/// OnWndProcNotify:
		/// 	Returns 0 if message was processed.
		///----------------------------------------------------------------------------------------------------
		UINT OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam) override;

		///----------------------------------------------------------------------------------------------------
		/// OnDxgiPresentNotify:
		/// 	Renders the UI.
		///----------------------------------------------------------------------------------------------------
		void OnDxgiPresentNotify(IDXGISwapChain* aSwapChain) override;

		///----------------------------------------------------------------------------------------------------
		/// OnDxgiResizeNotify:
		/// 	Shuts the UI down before the buffers are resized.
		///----------------------------------------------------------------------------------------------------
		void OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight) override;

		///----------------------------------------------------------------------------------------------------
		/// OnMumbleIdentityNotify:
		/// 	Resizes the fonts when the game UI size changed.
		///----------------------------------------------------------------------------------------------------
		void OnMumbleIdentityNotify(const Mumble::Identity& aIdentity) override;

		///----------------------------------------------------------------------------------------------------
		/// OnInputBindUpdateNotify:
		/// 	Refreshes the UI.
		///----------------------------------------------------------------------------------------------------
		void OnInputBindUpdateNotify() override;

		///----------------------------------------------------------------------------------------------------
		/// OnFontUpdateNotify:
		/// 	Applies the Nexus fonts to ImGui and NexusLink.
		///----------------------------------------------------------------------------------------------------
		void OnFontUpdateNotify(const char* aIdentifier, ImFont* aFont) override;

		///----------------------------------------------------------------------------------------------------
		/// GetLocalization:
		/// 	Returns the localization component.
		///----------------------------------------------------------------------------------------------------
		UI::CLocalization* GetLocalization();

		///----------------------------------------------------------------------------------------------------
		/// GetAlerts:
		/// 	Returns the alerts component.
		///----------------------------------------------------------------------------------------------------
		UI::CAlerts* GetAlerts();

		///----------------------------------------------------------------------------------------------------
		/// GetQuickAccess:
		/// 	Returns the quick access component.
		///----------------------------------------------------------------------------------------------------
		UI::CQuickAccess* GetQuickAccess();

		///----------------------------------------------------------------------------------------------------
		/// GetFontManager:
		/// 	Returns the font manager.
		///----------------------------------------------------------------------------------------------------
		UI::CFontManager* GetFontManager();

		///----------------------------------------------------------------------------------------------------
		/// GetEscapeClosingService:
		/// 	Returns the escape-closing service.
		///----------------------------------------------------------------------------------------------------
		UI::CEscapeClosing* GetEscapeClosingService();

		///----------------------------------------------------------------------------------------------------
		/// GetMainWindow:
		/// 	Returns the main window.
		///----------------------------------------------------------------------------------------------------
		UI::CMainWindow* GetMainWindow();

		///----------------------------------------------------------------------------------------------------
		/// GetScaling:
		/// 	Returns the scaling service.
		///----------------------------------------------------------------------------------------------------
		UI::CScaling* GetScaling();

		///----------------------------------------------------------------------------------------------------
		/// GetInput:
		/// 	Returns the UI input service.
		///----------------------------------------------------------------------------------------------------
		CUiInput* GetInput();

		///----------------------------------------------------------------------------------------------------
		/// LoadFonts:
		/// 	Loads the fonts.
		///----------------------------------------------------------------------------------------------------
		void LoadFonts();

		///----------------------------------------------------------------------------------------------------
		/// ApplyFontScale:
		/// 	Resizes the Nexus fonts to the game's UI scale. NexusLink keeps the previous fonts until the
		/// 	resized ones are built.
		///----------------------------------------------------------------------------------------------------
		void ApplyFontScale();

		private:
		/* Services */
		UI::RenderContext_t*        RenderContext  = nullptr;
		UI::CLocalization*          Language       = nullptr;
		UI::CTextureLoader*         TextureService = nullptr;
		Engine::CDataLinkApi*           DataLink       = nullptr;
		Engine::Input::CInputBindApi*   InputBindApi   = nullptr;
		GW2::CMumbleReader*          MumbleReader   = nullptr;

		/* Rendering */
		ID3D11RenderTargetView* RenderTargetView;

		/* Windows/Widgets */
		UI::CAlerts*                Alerts;
		UI::CMainWindow*            MainWindow;
		UI::CQuickAccess*           QuickAccess;

		/* UI Services */
		UI::CFontManager*           FontManager;
		UI::CEscapeClosing*         EscapeClose;
		UI::CScaling*               Scaling;
		CUiInput*               Input;

		bool                    IsInitialized = false;
		bool                    IsVisible = true;
		bool                    IsInvalid = true;

		public:
		Core::CSetting<bool>          EulaAccepted{ "AcceptEULA", false };
	};
}
