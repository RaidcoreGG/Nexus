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

#include "Engine/_Concepts/IWndProc.h"
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
#include "UI/Services/Scaling/Scaling.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/EULA/LicenseAgreementModal.h"
#include "UI/Widgets/MainWindow/MainWindow.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"
#include "UiBinds.h"
#include "UiInput.h"
#include "UiRender.h"
#include "UiStyle.h"
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
class CUiContext : public CUiRender, public CUiBinds, public CUiStyle, public virtual IWndProc
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
	/// OnMumbleIdentityChanged:
	/// 	Changes fonts and UI scale when the game settings are changed.
	///----------------------------------------------------------------------------------------------------
	static void OnMumbleIdentityChanged(void* aEventArgs);

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
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
	/// LoadFonts:
	/// 	Loads the fonts.
	///----------------------------------------------------------------------------------------------------
	void LoadFonts();

	private:
	/* Services */
	RenderContext_t*        RenderContext  = nullptr;
	CLogApi*                Logger         = nullptr;
	CLocalization*          Language       = nullptr;
	CTextureLoader*         TextureService = nullptr;
	CDataLinkApi*           DataLink       = nullptr;
	CInputBindApi*          InputBindApi   = nullptr;
	CEventApi*              EventApi       = nullptr;
	CMumbleReader*          MumbleReader   = nullptr;

	/* Rendering */
	ID3D11RenderTargetView* RenderTargetView;

	/* Windows/Widgets */
	CAlerts*                Alerts;
	CMainWindow*            MainWindow;
	CQuickAccess*           QuickAccess;

	/* UI Services */
	CFontManager*           FontManager;
	CEscapeClosing*         EscapeClose;
	CScaling*               Scaling;
	CUiInput*               Input;

	bool                    IsInitialized = false;
	bool                    IsVisible = true;
	bool                    IsInvalid = true;
};
