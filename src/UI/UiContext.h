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

#include "DisplayBinds.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Renderer/RdrContext.h"
#include "Engine/Textures/TxLoader.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/Services/Localization/LoclApi.h"
#include "UI/Services/QoL/EscapeClosing.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/EULA/LicenseAgreementModal.h"
#include "UI/Widgets/MainWindow/MainWindow.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"
#include "UiRender.h"
#include "Util/Inputs.h"

constexpr const char* CH_UICONTEXT       = "UI Context";
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
class CUiContext : public CUiRender
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnInputBindPressed:
	/// 	Receives input bind invocations.
	///----------------------------------------------------------------------------------------------------
	static void OnInputBindPressed(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// OnFontUpdate:
	/// 	Receives fonts.
	///----------------------------------------------------------------------------------------------------
	static void OnFontUpdate(const char* aIdentifier, ImFont* aFont);

	///----------------------------------------------------------------------------------------------------
	/// OnUELanguageChanged:
	/// 	Receives runtime language updates from unofficial extras.
	///----------------------------------------------------------------------------------------------------
	static void OnUELanguageChanged(uint32_t* aLanguage);

	///----------------------------------------------------------------------------------------------------
	/// OnMumbleIdentityChanged:
	/// 	Changes fonts and UI scale when the game settings are changed.
	///----------------------------------------------------------------------------------------------------
	static void OnMumbleIdentityChanged(void* aEventArgs);

	///----------------------------------------------------------------------------------------------------
	/// OnVolatileAddonsDisabled:
	/// 	Receiver for volatile addons disabled event to bring up the addons window.
	///----------------------------------------------------------------------------------------------------
	static void OnVolatileAddonsDisabled(void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// OnInputBindUpdate:
	/// 	Reacts to input bind changes, to trigger a UI refresh.
	///----------------------------------------------------------------------------------------------------
	static void OnInputBindUpdate(void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CUiContext(
		RenderContext_t* aRenderContext,
		CLogApi*         aLogger,
		CTextureLoader*  aTextureService,
		CDataLinkApi*    aDataLink,
		CInputBindApi*   aInputBindApi,
		CEventApi*       aEventApi,
		CMumbleReader*   aMumbleReader
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
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// OnInputBind:
	/// 	Invokes an input bind.
	///----------------------------------------------------------------------------------------------------
	void OnInputBind(std::string aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// UpdateScaling:
	/// 	Updates the UI scaling.
	///----------------------------------------------------------------------------------------------------
	void UpdateScaling();

	///----------------------------------------------------------------------------------------------------
	/// GetLocalization:
	/// 	Returns the localization component.
	///----------------------------------------------------------------------------------------------------
	CLocalization* GetLocalization();

	///----------------------------------------------------------------------------------------------------
	/// GetAlerts:
	/// 	Returns the alerts component.
	///----------------------------------------------------------------------------------------------------
	CAlerts* GetAlerts();

	///----------------------------------------------------------------------------------------------------
	/// GetQuickAccess:
	/// 	Returns the quick access component.
	///----------------------------------------------------------------------------------------------------
	CQuickAccess* GetQuickAccess();

	///----------------------------------------------------------------------------------------------------
	/// GetFontManager:
	/// 	Returns the font manager.
	///----------------------------------------------------------------------------------------------------
	CFontManager* GetFontManager();

	///----------------------------------------------------------------------------------------------------
	/// GetEscapeClosingService:
	/// 	Returns the escape-closing service.
	///----------------------------------------------------------------------------------------------------
	CEscapeClosing* GetEscapeClosingService();

	///----------------------------------------------------------------------------------------------------
	/// GetInputBinds:
	/// 	Returns a copy of the display input binds.
	///----------------------------------------------------------------------------------------------------
	std::vector<InputBindCategory_t> GetInputBinds();

	///----------------------------------------------------------------------------------------------------
	/// GetInputBinds:
	/// 	Returns a copy of the display input binds.
	///----------------------------------------------------------------------------------------------------
	std::unordered_map<std::string, InputBindPacked_t> GetInputBinds(const std::string& aCategory);

	///----------------------------------------------------------------------------------------------------
	/// GetGameBinds:
	/// 	Returns a copy of the display game input binds.
	///----------------------------------------------------------------------------------------------------
	std::vector<GameInputBindCategory_t> GetGameBinds();

	///----------------------------------------------------------------------------------------------------
	/// LoadFonts:
	/// 	Loads the fonts.
	///----------------------------------------------------------------------------------------------------
	void LoadFonts();

	///----------------------------------------------------------------------------------------------------
	/// ApplyStyle:
	/// 	Applies an ImGui style.
	/// 	aValue is either a filename or a code, if the style is EUIStyle::File or EUIStyle::Code.
	///----------------------------------------------------------------------------------------------------
	void ApplyStyle(EUIStyle aStyle = EUIStyle::User, std::string aValue = "");

	private:
	/* Services */
	RenderContext_t*                   RenderContext  = nullptr;
	CLogApi*                           Logger         = nullptr;
	CLocalization*                     Language       = nullptr;
	CTextureLoader*                    TextureService = nullptr;
	CDataLinkApi*                      DataLink       = nullptr;
	CInputBindApi*                     InputBindApi   = nullptr;
	CEventApi*                         EventApi       = nullptr;
	CMumbleReader*                     MumbleReader   = nullptr;

	/* Rendering */
	ID3D11RenderTargetView*            RenderTargetView;
	ImGuiContext*                      ImGuiContext;

	/* Windows/Widgets */
	CAlerts*                           Alerts;
	CMainWindow*                       MainWindow;
	CQuickAccess*                      QuickAccess;

	/* UI Services */
	CFontManager*                      FontManager;
	CEscapeClosing*                    EscapeClose;

	mutable std::mutex                 DisplayBindsMutex;
	std::vector<InputBindCategory_t>     DisplayInputBinds;
	std::vector<GameInputBindCategory_t> DisplayGameBinds;

	bool                               IsInitialized = false;
	bool                               IsVisible = true;
	bool                               IsInvalid = true;
	bool                               ClickingRequiresMods = false;
	bool                               AreModsDown = false;
	EModifiers                         Mods = EModifiers::None;

	///----------------------------------------------------------------------------------------------------
	/// UnpackLocales:
	/// 	Unpacks the default locale files.
	///----------------------------------------------------------------------------------------------------
	void UnpackLocales();

	///----------------------------------------------------------------------------------------------------
	/// LoadSettings:
	/// 	Loads all the UI settings.
	///----------------------------------------------------------------------------------------------------
	void LoadSettings();

	///----------------------------------------------------------------------------------------------------
	/// UpdateDisplayInputBinds:
	/// 	Refreshes the displayed input binds.
	///----------------------------------------------------------------------------------------------------
	void UpdateDisplayInputBinds();

	///----------------------------------------------------------------------------------------------------
	/// UpdateDisplayGameBinds:
	/// 	Refreshes the displayed game input binds.
	///----------------------------------------------------------------------------------------------------
	void UpdateDisplayGameBinds();
};
