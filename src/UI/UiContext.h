///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiContext.h
/// Description  :  Contains the core functionality of the User Interface.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef UICONTEXT_H
#define UICONTEXT_H

#include <d3d11.h>
#include <map>
#include <vector>
#include <Windows.h>

#include "DisplayBinds.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Renderer/RdrContext.h"
#include "Engine/Textures/TxLoader.h"
#include "ERenderType.h"
#include "FuncDefs.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/Services/Localization/LoclApi.h"
#include "UI/Services/QoL/EscapeClosing.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/EULA/LicenseAgreementModal.h"
#include "UI/Widgets/MainWindow/MainWindow.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"

constexpr const char* CH_UICONTEXT       = "UI Context";
constexpr const char* ICON_NEXUS         = "ICON_NEXUS";
constexpr const char* ICON_NEXUS_HOVER   = "ICON_NEXUS_HOVER";
constexpr const char* ICON_GENERIC       = "ICON_GENERIC";
constexpr const char* ICON_GENERIC_HOVER = "ICON_GENERIC_HOVER";
constexpr const char* KB_MENU            = "KB_MENU";
constexpr const char* KB_ADDONS          = "KB_ADDONS";
constexpr const char* KB_OPTIONS         = "KB_OPTIONS";
constexpr const char* KB_LOG             = "KB_LOG";
constexpr const char* KB_DEBUG           = "KB_DEBUG";
constexpr const char* KB_MUMBLEOVERLAY   = "KB_MUMBLEOVERLAY";
constexpr const char* KB_TOGGLEHIDEUI    = "KB_TOGGLEHIDEUI";

enum class EUIStyle
{
	User,
	Nexus,
	ImGui_Classic,
	ImGui_Light,
	ImGui_Dark,
	ArcDPS_Default,
	ArcDPS_Current, /* If available. */
	File,
	Code
};

///----------------------------------------------------------------------------------------------------
/// UIRoot Namespace
///----------------------------------------------------------------------------------------------------
namespace UIRoot
{
	extern float   ScalingFactor; /* DPI aware UI scaling factor matching ingame behavior. */

	extern ImFont* UserFont;      /* custom user font or default font */
	extern ImFont* Font;          /* Menomonia */
	extern ImFont* FontBig;       /* Menomonia, but slightly bigger */
	extern ImFont* FontUI;        /* Trebuchet */

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Initializes the UIRoot for static functions.
	///----------------------------------------------------------------------------------------------------
	void Initialize(CLocalization* aLocalization, CDataLinkApi* aDataLink, CFontManager* aFontManager);

	///----------------------------------------------------------------------------------------------------
	/// FontReceiver:
	/// 	Receives fonts.
	///----------------------------------------------------------------------------------------------------
	void FontReceiver(const char* aIdentifier, ImFont* aFont);

	///----------------------------------------------------------------------------------------------------
	/// OnUELanguageChanged:
	/// 	Receives runtime language updates from unofficial extras.
	///----------------------------------------------------------------------------------------------------
	void OnUELanguageChanged(void* aEventArgs);

	///----------------------------------------------------------------------------------------------------
	/// OnMumbleIdentityChanged:
	/// 	Changes fonts and UI scale when the game settings are changed.
	///----------------------------------------------------------------------------------------------------
	void OnMumbleIdentityChanged(void* aEventArgs);

	///----------------------------------------------------------------------------------------------------
	/// OnInputBind:
	/// 	Receives input bind invocations.
	///----------------------------------------------------------------------------------------------------
	void OnInputBind(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// OnVolatileAddonsDisabled:
	/// 	Receiver for volatile addons disabled event to bring up the addons window.
	///----------------------------------------------------------------------------------------------------
	void OnVolatileAddonsDisabled(void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// OnInputBindUpdate:
	/// 	Reacts to input bind changes, to trigger a UI refresh.
	///----------------------------------------------------------------------------------------------------
	void OnInputBindUpdate(void* aEventData);
}

///----------------------------------------------------------------------------------------------------
/// CUiContext Class
///----------------------------------------------------------------------------------------------------
class CUiContext
{
	public:
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
	/// Register:
	/// 	Registers the provided Render callback.
	///----------------------------------------------------------------------------------------------------
	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters the provided Render callback.
	///----------------------------------------------------------------------------------------------------
	void Deregister(GUI_RENDER aRenderCallback);

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all registered render callbacks and close-on-escape hooks that match the address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

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
	/// GetOptionsCallbacks:
	/// 	Returns a copy of the options callbacks.
	///----------------------------------------------------------------------------------------------------
	std::vector<GUI_RENDER> GetOptionsCallbacks();

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

	mutable std::mutex                 Mutex;
	std::vector<GUI_RENDER>            RegistryPreRender;
	std::vector<GUI_RENDER>            RegistryRender;
	std::vector<GUI_RENDER>            RegistryPostRender;
	std::vector<GUI_RENDER>            RegistryOptionsRender;

	mutable std::mutex                 DisplayBindsMutex;
	std::vector<InputBindCategory_t>     DisplayInputBinds;
	std::vector<GameInputBindCategory_t> DisplayGameBinds;

	bool                               IsInitialized = false;
	bool                               IsVisible = true;
	bool                               IsInvalid = true;

	///----------------------------------------------------------------------------------------------------
	/// CreateNexusShortcut:
	/// 	Creates Nexus own shortcut icon and adds it to the HUD.
	///----------------------------------------------------------------------------------------------------
	void CreateNexusShortcut();

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

#endif
