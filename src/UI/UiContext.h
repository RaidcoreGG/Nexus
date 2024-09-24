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
#include <Windows.h>

#include "ERenderType.h"
#include "FuncDefs.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Localization/Localization.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Textures/TextureLoader.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/Services/QoL/EscapeClosing.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/EULA/EULAModal.h"
#include "UI/Widgets/MainWindow/MainWindow.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"

constexpr const char* CH_UICONTEXT = "UI Context";
constexpr const char* ICON_NEXUS = "ICON_NEXUS";
constexpr const char* ICON_NEXUS_HOVER = "ICON_NEXUS_HOVER";
constexpr const char* ICON_GENERIC = "ICON_GENERIC";
constexpr const char* ICON_GENERIC_HOVER = "ICON_GENERIC_HOVER";

///----------------------------------------------------------------------------------------------------
/// UIRoot Namespace
///----------------------------------------------------------------------------------------------------
namespace UIRoot
{
	extern ImFont* MonospaceFont;	/* default/monospace/console font */
	extern ImFont* UserFont;		/* custom user font */
	extern ImFont* Font;			/* Menomonia */
	extern ImFont* FontBig;			/* Menomonia, but slightly bigger */
	extern ImFont* FontUI;			/* Trebuchet */

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Initializes the UIRoot for static functions.
	///----------------------------------------------------------------------------------------------------
	void Initialize(CLocalization* aLocalization, CDataLink* aDataLink, CFontManager* aFontManager);

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
	CUiContext(CLogHandler* aLogger, CLocalization* aLocalization, CTextureLoader* aTextureService, CDataLink* aDataLink, CInputBindApi* aInputBindApi);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CUiContext();

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Initializes the UI context.
	///----------------------------------------------------------------------------------------------------
	void Initialize(HWND aWindowHandle, ID3D11Device* aDevice, ID3D11DeviceContext* aDeviceContext, IDXGISwapChain* aSwapChain);

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

	private:
	CLogHandler* Logger;
	CLocalization* Language;
	CTextureLoader* TextureService;

	HWND WindowHandle;
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderTargetView;
	ImGuiContext* ImGuiContext;

	CEULAModal* EULAModal;
	CAlerts* Alerts;
	CMainWindow* MainWindow;
	CQuickAccess* QuickAccess;

	CFontManager* FontManager;
	CEscapeClosing* EscapeClose;

	mutable std::mutex Mutex;
	std::vector<GUI_RENDER> RegistryPreRender;
	std::vector<GUI_RENDER> RegistryRender;
	std::vector<GUI_RENDER> RegistryPostRender;
	std::vector<GUI_RENDER> RegistryOptionsRender;

	bool IsInitialized = false;
	bool IsVisible = true;

	bool IsRightClickHeld = false;
	bool IsLeftClickHeld = false;

	///----------------------------------------------------------------------------------------------------
	/// CreateNexusShortcut:
	/// 	Creates Nexus own shortcut icon and adds it to the HUD.
	///----------------------------------------------------------------------------------------------------
	void CreateNexusShortcut();

	///----------------------------------------------------------------------------------------------------
	/// ApplyStyle:
	/// 	Applies the ImGui style.
	///----------------------------------------------------------------------------------------------------
	void ApplyStyle();

	///----------------------------------------------------------------------------------------------------
	/// UnpackLocales:
	/// 	Unpacks the default locale files.
	///----------------------------------------------------------------------------------------------------
	void UnpackLocales();

	///----------------------------------------------------------------------------------------------------
	/// LoadFonts:
	/// 	Loads the fonts.
	///----------------------------------------------------------------------------------------------------
	void LoadFonts();

	/* FIXME: dirty, this shit needs to be done properly */
	std::string FontFile;
	bool LinkArcDPSStyle = true;
	void LoadSettings();
};

// FIXME: addon wrapper garbage
namespace UIRoot::GUI
{
	extern CUiContext* UICtx;

	void ADDONAPI_Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);

	void ADDONAPI_Deregister(GUI_RENDER aRenderCallback);
}
namespace UIRoot::Fonts
{
	extern CFontManager* FontCtx;

	void ADDONAPI_Get(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

	void ADDONAPI_Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

	void ADDONAPI_AddFontFromFile(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	void ADDONAPI_AddFontFromResource(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	void ADDONAPI_AddFontFromMemory(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	void ADDONAPI_ResizeFont(const char* aIdentifier, float aFontSize);
}
namespace UIRoot::Alerts
{
	extern CAlerts* AlertCtx;

	void ADDONAPI_Notify(const char* aMessage);
}
namespace UIRoot::QuickAccess
{
	extern CQuickAccess* QACtx;

	void ADDONAPI_AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);

	void ADDONAPI_RemoveShortcut(const char* aIdentifier);

	void ADDONAPI_NotifyShortcut(const char* aIdentifier);

	void ADDONAPI_SetNotificationShortcut(const char* aIdentifier, bool aState);

	void ADDONAPI_AddContextItem(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);

	void ADDONAPI_AddContextItem2(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback);

	void ADDONAPI_RemoveContextItem(const char* aIdentifier);
}
namespace UIRoot::EscapeClosing
{
	extern CEscapeClosing* EscCtx;

	void ADDONAPI_Register(const char* aWindowName, bool* aIsVisible);

	void ADDONAPI_Deregister(const char* aWindowName);
}

#endif
