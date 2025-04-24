///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiContext.cpp
/// Description  :  Contains the core functionality of the User Interface.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UiContext.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

#include "Consts.h"
#include "Context.h"
#include "Index.h"
#include "Renderer.h"
#include "resource.h"
#include "Services/Mumble/Reader.h"
#include "Services/Settings/Settings.h"
#include "Shared.h"
#include "State.h"
#include "Util/Base64.h"
#include "Util/Inputs.h"
#include "Util/Resources.h"
#include "Util/Time.h"
#include "Util/Strings.h"
#include "Inputs/GameBinds/GbConst.h"

#include "UI/Widgets/MainWindow/About/About.h"
#include "UI/Widgets/MainWindow/Addons/Addons.h"
#include "UI/Widgets/MainWindow/Binds/Binds.h"
#include "UI/Widgets/MainWindow/Debug/Debug.h"
#include "UI/Widgets/MainWindow/Log/Log.h"
#include "UI/Widgets/MainWindow/Options/Options.h"

namespace UIRoot
{
	ImFont* MonospaceFont            = nullptr;
	ImFont* UserFont                 = nullptr;
	ImFont* Font                     = nullptr;
	ImFont* FontBig                  = nullptr;
	ImFont* FontUI                   = nullptr;

	CLocalization* Language          = nullptr;
	CFontManager* FontManager        = nullptr;
	Mumble::Identity* MumbleIdentity = nullptr;
	NexusLinkData* NexusLink         = nullptr;

	void Initialize(CLocalization* aLocalization, CDataLinkApi* aDataLink, CFontManager* aFontManager)
	{
		Language = aLocalization;
		FontManager = aFontManager;
		MumbleIdentity = (Mumble::Identity*)aDataLink->GetResource(DL_MUMBLE_LINK_IDENTITY);
		NexusLink = (NexusLinkData*)aDataLink->GetResource(DL_NEXUS_LINK);
	}

	void FontReceiver(const char* aIdentifier, ImFont* aFont)
	{
		std::string str = aIdentifier;

		if (str == "USER_FONT")
		{
			UserFont = aFont;

			ImGuiIO& io = ImGui::GetIO();
			io.FontDefault = UserFont;
		}
		else if (str == "FONT_DEFAULT")
		{
			MonospaceFont = aFont;
		}
		else
		{
			/* directly assign the font */
			switch (MumbleIdentity->UISize)
			{
				case Mumble::EUIScale::Small:
				{
					if (str == "MENOMONIA_S") { NexusLink->Font = Font = aFont; }
					else if (str == "MENOMONIA_BIG_S") { NexusLink->FontBig = FontBig = aFont; }
					else if (str == "TREBUCHET_S") { NexusLink->FontUI = FontUI = aFont; }

					break;
				}
				default:
				case Mumble::EUIScale::Normal:
				{
					if (str == "MENOMONIA_N") { NexusLink->Font = Font = aFont; }
					else if (str == "MENOMONIA_BIG_N") { NexusLink->FontBig = FontBig = aFont; }
					else if (str == "TREBUCHET_N") { NexusLink->FontUI = FontUI = aFont; }

					break;
				}
				case Mumble::EUIScale::Large:
				{
					if (str == "MENOMONIA_L") { NexusLink->Font = Font = aFont; }
					else if (str == "MENOMONIA_BIG_L") { NexusLink->FontBig = FontBig = aFont; }
					else if (str == "TREBUCHET_L") { NexusLink->FontUI = FontUI = aFont; }

					break;
				}
				case Mumble::EUIScale::Larger:
				{
					if (str == "MENOMONIA_XL") { NexusLink->Font = Font = aFont; }
					else if (str == "MENOMONIA_BIG_XL") { NexusLink->FontBig = FontBig = aFont; }
					else if (str == "TREBUCHET_XL") { NexusLink->FontUI = FontUI = aFont; }

					break;
				}
			}
		}
	}

	void OnUELanguageChanged(void* aEventArgs)
	{
		if (!Language)
		{
			return;
		}

		int langId = *(int*)aEventArgs;

		switch (langId)
		{
			case 0:
				Language->SetLanguage("en");
				break;
			case 2:
				Language->SetLanguage("fr");
				break;
			case 3:
				Language->SetLanguage("de");
				break;
			case 4:
				Language->SetLanguage("es");
				break;
			case 5:
				Language->SetLanguage("cn");
				break;
		}
	}

	void OnMumbleIdentityChanged(void* aEventArgs)
	{
		if (!MumbleIdentity || !NexusLink)
		{
			return;
		}

		CContext* ctx = CContext::GetContext();
		CSettings* settingsctx = ctx->GetSettingsCtx();

		float currScaling = Mumble::GetScalingFactor(MumbleIdentity->UISize);
		if (currScaling != settingsctx->Get<float>(OPT_LASTUISCALE, 1.0f) && NexusLink->IsGameplay)
		{
			ImGuiIO& io = ImGui::GetIO();

			settingsctx->Set(OPT_LASTUISCALE, currScaling);

			CUiContext* uictx = ctx->GetUIContext();
			uictx->UpdateScaling();
		}

		if (!FontManager)
		{
			return;
		}

		switch (MumbleIdentity->UISize)
		{
			case Mumble::EUIScale::Small:
			{
				NexusLink->Font = Font = FontManager->Get("MENOMONIA_S")->Pointer;
				NexusLink->FontBig = FontBig = FontManager->Get("MENOMONIA_BIG_S")->Pointer;
				NexusLink->FontUI = FontUI = FontManager->Get("TREBUCHET_S")->Pointer;

				break;
			}
			default:
			case Mumble::EUIScale::Normal:
			{
				NexusLink->Font = Font = FontManager->Get("MENOMONIA_N")->Pointer;
				NexusLink->FontBig = FontBig = FontManager->Get("MENOMONIA_BIG_N")->Pointer;
				NexusLink->FontUI = FontUI = FontManager->Get("TREBUCHET_N")->Pointer;

				break;
			}
			case Mumble::EUIScale::Large:
			{
				NexusLink->Font = Font = FontManager->Get("MENOMONIA_L")->Pointer;
				NexusLink->FontBig = FontBig = FontManager->Get("MENOMONIA_BIG_L")->Pointer;
				NexusLink->FontUI = FontUI = FontManager->Get("TREBUCHET_L")->Pointer;

				break;
			}
			case Mumble::EUIScale::Larger:
			{
				NexusLink->Font = Font = FontManager->Get("MENOMONIA_XL")->Pointer;
				NexusLink->FontBig = FontBig = FontManager->Get("MENOMONIA_BIG_XL")->Pointer;
				NexusLink->FontUI = FontUI = FontManager->Get("TREBUCHET_XL")->Pointer;

				break;
			}
		}
	}

	void OnInputBind(const char* aIdentifier)
	{
		CContext* ctx = CContext::GetContext();
		CUiContext* uictx = ctx->GetUIContext();

		assert(uictx);

		uictx->OnInputBind(aIdentifier);
	}

	void OnVolatileAddonsDisabled(void* aEventData)
	{
		CContext* ctx = CContext::GetContext();
		CUiContext* uictx = ctx->GetUIContext();
		CSettings* settingsctx = ctx->GetSettingsCtx();

		assert(uictx);
		assert(settingsctx);

		if (settingsctx->Get<bool>(OPT_SHOWADDONSWINDOWAFTERDUU, false))
		{
			uictx->OnInputBind(KB_ADDONS);
		}
	}

	void OnInputBindUpdate(void* aEventData)
	{
		CContext* ctx = CContext::GetContext();
		CUiContext* uictx = ctx->GetUIContext();

		if (uictx)
		{
			uictx->Invalidate();
		}
	}

	void OnLanguageChanged(void* aEventData)
	{
		CContext* ctx = CContext::GetContext();
		CUiContext* uictx = ctx->GetUIContext();

		assert(uictx);

		uictx->Invalidate();
	}
}

CUiContext::CUiContext(CLogHandler* aLogger, CLocalization* aLocalization, CTextureLoader* aTextureService, CDataLinkApi* aDataLink, CInputBindApi* aInputBindApi, CEventApi* aEventApi)
{
	this->Logger         = aLogger;
	this->Language       = aLocalization;
	this->TextureService = aTextureService;
	this->DataLink       = aDataLink;
	this->InputBindApi   = aInputBindApi;
	this->EventApi       = aEventApi;

	this->ImGuiContext   = ImGui::CreateContext();

	this->Alerts         = new CAlerts(aDataLink);
	this->MainWindow     = new CMainWindow();
	this->QuickAccess    = new CQuickAccess(aDataLink, aLogger, aInputBindApi, aTextureService, aLocalization, aEventApi);

	this->FontManager    = new CFontManager(aLocalization);
	this->EscapeClose    = new CEscapeClosing();

	UIRoot::Initialize(this->Language, this->DataLink, this->FontManager);

	this->EventApi->Subscribe("EV_MUMBLE_IDENTITY_UPDATED",            UIRoot::OnMumbleIdentityChanged);
	this->EventApi->Subscribe("EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED", UIRoot::OnUELanguageChanged);
	this->EventApi->Subscribe("EV_VOLATILE_ADDON_DISABLED",            UIRoot::OnVolatileAddonsDisabled);
	this->EventApi->Subscribe("EV_INPUTBIND_UPDATED",                  UIRoot::OnInputBindUpdate);
	this->EventApi->Subscribe("EV_LANGUAGE_CHANGED",                   UIRoot::OnLanguageChanged);

	CAddonsWindow*  addonsWnd  = new CAddonsWindow();
	COptionsWindow* optionsWnd = new COptionsWindow();
	CBindsWindow*   bindsWNd   = new CBindsWindow();
	CLogWindow*     logWnd     = new CLogWindow();
	CDebugWindow*   debugWnd   = new CDebugWindow();
	//CAboutBox*      aboutWnd   = new CAboutBox();

	this->Logger->RegisterLogger(logWnd);

	this->MainWindow->AddWindow(addonsWnd);
	this->MainWindow->AddWindow(optionsWnd);
	this->MainWindow->AddWindow(bindsWNd);
	this->MainWindow->AddWindow(logWnd);
	this->MainWindow->AddWindow(debugWnd);
	//this->MainWindow->AddWindow(aboutWnd);

	/* register InputBinds */
	this->InputBindApi->Register(KB_MENU,          EIBHType::DownOnly, UIRoot::OnInputBind, "CTRL+O");
	this->InputBindApi->Register(KB_ADDONS,        EIBHType::DownOnly, UIRoot::OnInputBind, NULLSTR);
	this->InputBindApi->Register(KB_OPTIONS,       EIBHType::DownOnly, UIRoot::OnInputBind, NULLSTR);
	this->InputBindApi->Register(KB_LOG,           EIBHType::DownOnly, UIRoot::OnInputBind, NULLSTR);
	this->InputBindApi->Register(KB_DEBUG,         EIBHType::DownOnly, UIRoot::OnInputBind, NULLSTR);
	this->InputBindApi->Register(KB_TOGGLEHIDEUI,  EIBHType::DownOnly, UIRoot::OnInputBind, "CTRL+H");

	this->EscapeClose->Register("Nexus", this->MainWindow->GetVisibleStatePtr());

	this->UnpackLocales();
	this->LoadSettings();
	this->CreateNexusShortcut();
	this->LoadFonts();
}

CUiContext::~CUiContext()
{
	this->Logger = nullptr;
	this->Language = nullptr;

	this->ImGuiContext = nullptr;
	ImGui::DestroyContext();
}

void CUiContext::Initialize(HWND aWindowHandle, ID3D11Device* aDevice, ID3D11DeviceContext* aDeviceContext, IDXGISwapChain* aSwapChain)
{
	if (this->IsInitialized)
	{
		return;
	}

	this->WindowHandle = aWindowHandle;
	this->Device = aDevice;
	this->DeviceContext = aDeviceContext;
	this->SwapChain = aSwapChain;

	// Init imgui
	ImGui_ImplWin32_Init(this->WindowHandle);
	ImGui_ImplDX11_Init(this->Device, this->DeviceContext);
	//ImGui::GetIO().ImeWindowHandle = Renderer::WindowHandle;

	// create buffers
	ID3D11Texture2D* pBackBuffer;
	this->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

	if (!pBackBuffer)
	{
		this->Logger->Critical(CH_UICONTEXT, "CUiContext::Initialize() failed. BackBuffer was nullptr.");
		return;
	}

	this->Device->CreateRenderTargetView(pBackBuffer, NULL, &this->RenderTargetView);
	pBackBuffer->Release();

	if (!this->RenderTargetView)
	{
		this->Logger->Critical(CH_UICONTEXT, "CUiContext::Initialize() failed. RenderTargetView could not be created.");
		return;
	}

	this->UpdateScaling();

	this->IsInitialized = true;
}

void CUiContext::Shutdown()
{
	if (!this->IsInitialized)
	{
		return;
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	if (this->RenderTargetView)
	{
		this->RenderTargetView->Release();
		this->RenderTargetView = 0;
	}

	this->IsInitialized = false;
}

static bool EulaAccepted = false;

void CUiContext::Render()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* preload localization and font changes */
	if (this->Language->Advance())
	{
		this->FontManager->Reload();
	}
	if (this->FontManager->Advance())
	{
		UIRoot::OnLanguageChanged(nullptr);
		UIRoot::OnMumbleIdentityChanged(nullptr);
		Shutdown();
	}

	if (this->IsInvalid)
	{
		this->UpdateDisplayInputBinds();
		this->UpdateDisplayGameBinds();

		//this->Alerts->Invalidate();
		this->MainWindow->Invalidate();
		this->QuickAccess->Invalidate();

		this->IsInvalid = false;
	}

	/* pre-render callbacks */
	for (GUI_RENDER callback : this->RegistryPreRender)
	{
		if (callback) { callback(); }
	}

	if (this->IsInitialized)
	{
		/* new frame */
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		/* draw overlay */
		if (EulaAccepted)
		{
			if (this->IsVisible)
			{
				/* draw addons*/
				for (GUI_RENDER callback : this->RegistryRender)
				{
					if (callback) { callback(); }
				}

				/* draw nexus windows */
				this->Alerts->Render();
				this->MainWindow->Render();
				this->QuickAccess->Render();
			}
		}
		else
		{
			/* only create eula modal when we actually need it */
			if (!this->EULAModal)
			{
				this->EULAModal = new CEULAModal(this->WindowHandle, this->Language);
			}

			/* if returns true, eula was accepted. free the memory */
			if (this->EULAModal->Render())
			{
				delete this->EULAModal;
				this->EULAModal = nullptr;

				CContext* ctx = CContext::GetContext();
				CSettings* settingsctx = ctx->GetSettingsCtx();
				settingsctx->Set(OPT_ACCEPTEULA, true);
				EulaAccepted = true;

				/* activate main window for the first time */
				this->MainWindow->Activate();
			}
		}

		/* end frame */
		ImGui::EndFrame();
		ImGui::Render();
		this->DeviceContext->OMSetRenderTargets(1, &this->RenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	/* post-render callbacks */
	for (GUI_RENDER callback : this->RegistryPostRender)
	{
		if (callback) { callback(); }
	}
}

void CUiContext::Invalidate()
{
	this->IsInvalid = true;
}

UINT CUiContext::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->IsInitialized)
	{
		return uMsg;
	}

	ImGuiIO& io = ImGui::GetIO();

	switch (uMsg)
	{
		// mouse input
		case WM_MOUSEMOVE:
			if (!Inputs::IsCursorHidden())
			{
				/* only set cursor pos if cursor is visible */
				io.MousePos = ImVec2((float)(LOWORD(lParam)), (float)(HIWORD(lParam)));
			}
			break;

		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				io.MouseDown[0] = true;
				return 0;
			}
			else //if (!io.WantCaptureMouse)
			{
				ImGui::ClearActiveID();
			}
			break;
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				io.MouseDown[1] = true;
				return 0;
			}
			break;

			// doesn't hurt passing these through to the game
		case WM_LBUTTONUP:
			io.MouseDown[0] = false;
			break;
		case WM_RBUTTONUP:
			io.MouseDown[1] = false;
			break;

			// scroll
		case WM_MOUSEWHEEL:
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
				return 0;
			}
			break;
		case WM_MOUSEHWHEEL:
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
				return 0;
			}
			break;

			// key input
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (io.WantTextInput)
			{
				if (wParam < 256)
					io.KeysDown[wParam] = true;
				return 0;
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = false;
			break;
		case WM_CHAR:
			if (io.WantTextInput)
			{
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacterUTF16((unsigned short)wParam);
				return 0;
			}
			break;
	}

	if (this->EscapeClose->WndProc(hWnd, uMsg, wParam, lParam) == 0)
	{
		return 0;
	}

	return uMsg;
}

void CUiContext::Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
{
	if (!aRenderCallback) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	switch (aRenderType)
	{
		case ERenderType::PreRender:
			this->RegistryPreRender.push_back(aRenderCallback);
			break;
		case ERenderType::Render:
			this->RegistryRender.push_back(aRenderCallback);
			break;
		case ERenderType::PostRender:
			this->RegistryPostRender.push_back(aRenderCallback);
			break;
		case ERenderType::OptionsRender:
			this->RegistryOptionsRender.push_back(aRenderCallback);
			break;
	}
}

void CUiContext::Deregister(GUI_RENDER aRenderCallback)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->RegistryPreRender.erase(std::remove(this->RegistryPreRender.begin(), this->RegistryPreRender.end(), aRenderCallback), this->RegistryPreRender.end());
	this->RegistryRender.erase(std::remove(this->RegistryRender.begin(), this->RegistryRender.end(), aRenderCallback), this->RegistryRender.end());
	this->RegistryPostRender.erase(std::remove(this->RegistryPostRender.begin(), this->RegistryPostRender.end(), aRenderCallback), this->RegistryPostRender.end());
	this->RegistryOptionsRender.erase(std::remove(this->RegistryOptionsRender.begin(), this->RegistryOptionsRender.end(), aRenderCallback), this->RegistryOptionsRender.end());
}

int CUiContext::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	for (GUI_RENDER renderCb : this->RegistryPreRender)
	{
		if (renderCb >= aStartAddress && renderCb <= aEndAddress)
		{
			this->RegistryPreRender.erase(std::remove(this->RegistryPreRender.begin(), this->RegistryPreRender.end(), renderCb), this->RegistryPreRender.end());
			refCounter++;
		}
	}
	for (GUI_RENDER renderCb : this->RegistryRender)
	{
		if (renderCb >= aStartAddress && renderCb <= aEndAddress)
		{
			this->RegistryRender.erase(std::remove(this->RegistryRender.begin(), this->RegistryRender.end(), renderCb), this->RegistryRender.end());
			refCounter++;
		}
	}
	for (GUI_RENDER renderCb : this->RegistryPostRender)
	{
		if (renderCb >= aStartAddress && renderCb <= aEndAddress)
		{
			RegistryPostRender.erase(std::remove(RegistryPostRender.begin(), RegistryPostRender.end(), renderCb), RegistryPostRender.end());
			refCounter++;
		}
	}
	for (GUI_RENDER renderCb : RegistryOptionsRender)
	{
		if (renderCb >= aStartAddress && renderCb <= aEndAddress)
		{
			this->RegistryOptionsRender.erase(std::remove(this->RegistryOptionsRender.begin(), this->RegistryOptionsRender.end(), renderCb), this->RegistryOptionsRender.end());
			refCounter++;
		}
	}

	return refCounter;
}

void CUiContext::OnInputBind(std::string aIdentifier)
{
	if (aIdentifier == KB_TOGGLEHIDEUI)
	{
		this->IsVisible = !this->IsVisible;
	}
	else if (aIdentifier == KB_MENU)
	{
		this->MainWindow->Activate();
	}
	else if (aIdentifier == KB_ADDONS)
	{
		this->MainWindow->Activate("Addons");
	}
	else if (aIdentifier == KB_DEBUG)
	{
		this->MainWindow->Activate("Debug");
	}
	else if (aIdentifier == KB_LOG)
	{
		this->MainWindow->Activate("Log");
	}
	else if (aIdentifier == KB_OPTIONS)
	{
		this->MainWindow->Activate("Options");
	}
}

void CUiContext::UpdateScaling()
{
	NexusLinkData* nexuslink = (NexusLinkData*)this->DataLink->GetResource(DL_NEXUS_LINK);
	
	ImGuiIO& io = ImGui::GetIO();

	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	float ingamescale = settingsctx->Get<float>(OPT_LASTUISCALE, 1.0f);

	Renderer::Scaling = ingamescale * io.FontGlobalScale
		* min(min(Renderer::Width, 1024.0) / 1024.0, min(Renderer::Height, 768.0) / 768.0);

	if (nexuslink)
	{
		nexuslink->Scaling = Renderer::Scaling;
	}
}

CAlerts* CUiContext::GetAlerts()
{
	return this->Alerts;
}

CQuickAccess* CUiContext::GetQuickAccess()
{
	return this->QuickAccess;
}

CFontManager* CUiContext::GetFontManager()
{
	return this->FontManager;
}

CEscapeClosing* CUiContext::GetEscapeClosingService()
{
	return this->EscapeClose;
}

std::vector<GUI_RENDER> CUiContext::GetOptionsCallbacks()
{
	// no lock here, only safe to call from within render callbacks
	//const std::lock_guard<std::mutex> lock(this->Mutex);
	return this->RegistryOptionsRender;
}

std::vector<InputBindCategory> CUiContext::GetInputBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	return this->DisplayInputBinds;
}

std::unordered_map<std::string, InputBindPacked> CUiContext::GetInputBinds(const std::string& aCategory)
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	auto it = std::find_if(this->DisplayInputBinds.begin(), this->DisplayInputBinds.end(), [aCategory](InputBindCategory cat) {return cat.Name == aCategory; });
	
	if (it != this->DisplayInputBinds.end())
	{
		return it->InputBinds;
	}
	
	return {};
}

std::vector<GameInputBindCategory> CUiContext::GetGameBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	return this->DisplayGameBinds;
}

void CUiContext::LoadFonts()
{
	std::filesystem::path fontPath{};

	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	EulaAccepted = settingsctx->Get<bool>(OPT_ACCEPTEULA, false);

	/* add user font */
	bool hasUserFont = false;

	std::string fontFile = settingsctx->Get<std::string>(OPT_USERFONT, "");
	if (!fontFile.empty() && std::filesystem::exists(Index::D_GW2_ADDONS_NEXUS_FONTS / fontFile))
	{
		fontPath = Index::D_GW2_ADDONS_NEXUS_FONTS / fontFile;
		this->FontManager->ReplaceFont("USER_FONT", this->ImGuiContext->FontSize, fontPath.string().c_str(), UIRoot::FontReceiver, nullptr);
		hasUserFont = true;
	}
	else if (settingsctx->Get<bool>(OPT_LINKARCSTYLE, true) && std::filesystem::exists(Index::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf"))
	{
		fontPath = Index::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf";
		this->FontManager->ReplaceFont("USER_FONT", this->ImGuiContext->FontSize, fontPath.string().c_str(), UIRoot::FontReceiver, nullptr);
		hasUserFont = true;
	}

	/* add default font for monospace */
	this->FontManager->AddDefaultFont(UIRoot::FontReceiver);

	if (!hasUserFont)
	{
		this->FontManager->ResizeFont("FONT_DEFAULT", this->ImGuiContext->FontSize);
	}

	ImFontConfig config;
	config.MergeMode = true;

	/* small UI*/
	this->FontManager->ReplaceFont("MENOMONIA_S", 16.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_S_MERGE", 16.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_S", 22.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_S_MERGE", 22.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("TREBUCHET_S", 15.0f, RES_FONT_TREBUCHET, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("TREBUCHET_S_MERGE", 15.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }

	/* normal UI*/
	this->FontManager->ReplaceFont("MENOMONIA_N", 18.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_N_MERGE", 18.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_N", 24.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_N_MERGE", 24.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("TREBUCHET_N", 16.0f, RES_FONT_TREBUCHET, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("TREBUCHET_N_MERGE", 16.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }

	/* large UI*/
	this->FontManager->ReplaceFont("MENOMONIA_L", 20.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_L_MERGE", 20.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_L", 26.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_L_MERGE", 26.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("TREBUCHET_L", 17.5f, RES_FONT_TREBUCHET, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("TREBUCHET_L_MERGE", 17.5f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }

	/* larger UI*/
	this->FontManager->ReplaceFont("MENOMONIA_XL", 22.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_XL_MERGE", 22.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_XL", 28.0f, RES_FONT_MENOMONIA, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_XL_MERGE", 28.0f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
	this->FontManager->ReplaceFont("TREBUCHET_XL", 19.5f, RES_FONT_TREBUCHET, ctx->GetModule(), UIRoot::FontReceiver, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("TREBUCHET_XL_MERGE", 19.5f, fontPath.string().c_str(), UIRoot::FontReceiver, &config); }
}

void CUiContext::ApplyStyle(EUIStyle aStyle)
{
	ImGuiStyle* style = &ImGui::GetStyle();

	switch (aStyle)
	{
		case EUIStyle::ImGui_Classic:
		{
			ImGui::StyleColorsClassic();
			return;
		}
		case EUIStyle::ImGui_Light:
		{
			ImGui::StyleColorsLight();
			return;
		}
		case EUIStyle::ImGui_Dark:
		{
			ImGui::StyleColorsDark();
			return;
		}
		case EUIStyle::Nexus:
		{
			ImVec4* colors = style->Colors;

			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
			colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
			colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
			colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
			colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
			return;
		}
	}

	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	if (settingsctx->Get<bool>(OPT_LINKARCSTYLE, true))
	{
		std::filesystem::path arcIniPath = Index::D_GW2_ADDONS / "arcdps/arcdps.ini";

		if (std::filesystem::exists(arcIniPath))
		{
			std::ifstream arcIni(arcIniPath);

			if (arcIni.is_open())
			{
				std::string line;
				std::string styleKey = "appearance_imgui_style180=";
				std::string coloursKey = "appearance_imgui_colours180=";
				std::string fontSizeKey = "font_size=";

				try
				{
					while (std::getline(arcIni, line))
					{
						if (line.find(styleKey, 0) != line.npos)
						{
							line = line.substr(styleKey.length());
							std::string decode = Base64::Decode(&line[0], line.length());
							memcpy(style, &decode[0], decode.length());
						}
						else if (line.find(coloursKey, 0) != line.npos)
						{
							line = line.substr(coloursKey.length());
							std::string decode = Base64::Decode(&line[0], line.length());
							memcpy(&style->Colors[0], &decode[0], decode.length());
						}
						else if (line.find(fontSizeKey, 0) != line.npos)
						{
							line = line.substr(fontSizeKey.length());
							this->ImGuiContext->FontSize = std::stof(line);
						}
					}
				}
				catch (...)
				{
					this->Logger->Debug(CH_UICONTEXT, "Couldn't parse ArcDPS style.");
				}

				arcIni.close();
			}
		}
	}
	else
	{
		std::string b64_style = settingsctx->Get<std::string>(OPT_IMGUISTYLE, {});
		std::string b64_colors = settingsctx->Get<std::string>(OPT_IMGUICOLORS, {});

		if (b64_style.empty() || b64_colors.empty())
		{
			this->ApplyStyle(EUIStyle::Nexus);
		}

		{
			std::string decode = Base64::Decode(&b64_style[0], b64_style.length());
			memcpy_s(style, sizeof(ImGuiStyle), &decode[0], decode.length());
		}

		{
			std::string decode = Base64::Decode(&b64_colors[0], b64_colors.length());
			memcpy_s(&style->Colors[0], sizeof(ImVec4) * ImGuiCol_COUNT, &decode[0], decode.length());
		}
	}
}

void CUiContext::CreateNexusShortcut()
{
	int month = Time::GetMonth();

	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	bool isPartyPooper = settingsctx->Get<bool>(OPT_DISABLEFESTIVEFLAIR, false);

	if (isPartyPooper)
	{
		month = 0;
	}
	
	int resIcon = 0;
	int resIconHover = 0;

	switch (month)
	{
		case 10:
			resIcon = RES_ICON_NEXUS_HALLOWEEN;
			resIconHover = RES_ICON_NEXUS_HALLOWEEN_HOVER;
			break;
		case 12:
			resIcon = RES_ICON_NEXUS_XMAS;
			resIconHover = RES_ICON_NEXUS_XMAS_HOVER;
			break;
		default:
			resIcon = RES_ICON_NEXUS;
			resIconHover = RES_ICON_NEXUS_HOVER;
			break;
	}

	this->TextureService->Load(ICON_NEXUS, resIcon, ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_NEXUS_HOVER, resIconHover, ctx->GetModule(), nullptr);

	this->TextureService->Load(ICON_GENERIC, RES_ICON_GENERIC, ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_GENERIC_HOVER, RES_ICON_GENERIC_HOVER, ctx->GetModule(), nullptr);

	/* add shortcut */
	this->QuickAccess->AddShortcut(QA_MENU, ICON_NEXUS, ICON_NEXUS_HOVER, KB_MENU, "((000009))");
}

void CUiContext::UnpackLocales()
{
	CContext* ctx = CContext::GetContext();

	this->Language->SetLocaleDirectory(Index::D_GW2_ADDONS_NEXUS_LOCALES);
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_EN, RES_LOCALE_EN, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_DE, RES_LOCALE_DE, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_FR, RES_LOCALE_FR, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_ES, RES_LOCALE_ES, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_BR, RES_LOCALE_BR, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_CZ, RES_LOCALE_CZ, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_IT, RES_LOCALE_IT, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_PL, RES_LOCALE_PL, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_RU, RES_LOCALE_RU, "JSON");
	Resources::Unpack(ctx->GetModule(), Index::F_LOCALE_CN, RES_LOCALE_CN, "JSON");
	this->Language->Advance();
}

void CUiContext::LoadSettings()
{
	CContext* ctx = CContext::GetContext();
	CSettings* settingsCtx = ctx->GetSettingsCtx();

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle* style = &ImGui::GetStyle();

	float storedFontSz = settingsCtx->Get<float>(OPT_FONTSIZE, 13.0f);
	this->ImGuiContext->FontSize = storedFontSz > 0 ? storedFontSz : 13.0f;

	this->ApplyStyle();
	
	std::string lang = settingsCtx->Get<std::string>(OPT_LANGUAGE, "en");
	Language->SetLanguage(!lang.empty() ? lang : "en");

	float storedGlobalFontScale = settingsCtx->Get<float>(OPT_GLOBALSCALE, 1.0f);
	io.FontGlobalScale = storedGlobalFontScale > 0 ? storedGlobalFontScale : 1.0f;

	float lastUiScale = settingsCtx->Get<float>(OPT_LASTUISCALE, 1.0f);
	if (lastUiScale == 0)
	{
		lastUiScale = SC_NORMAL;
		settingsCtx->Set(OPT_LASTUISCALE, SC_NORMAL);
	}
	
	this->UpdateScaling();
}

void CUiContext::UpdateDisplayInputBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	this->DisplayInputBinds.clear();

	CContext* ctx = CContext::GetContext();
	CInputBindApi* inputBindApi = ctx->GetInputBindApi();

	/* copy of all InputBinds */
	std::map<std::string, ManagedInputBind> InputBindRegistry = inputBindApi->GetRegistry();

	/* acquire categories */
	for (auto& [identifier, inputBind] : InputBindRegistry)
	{
		std::string owner = Loader::GetOwner(inputBind.Handler);

		auto it = std::find_if(this->DisplayInputBinds.begin(), this->DisplayInputBinds.end(), [owner](InputBindCategory category) { return category.Name == owner; });

		if (it == this->DisplayInputBinds.end())
		{
			InputBindCategory cat{};
			cat.Name = owner;
			cat.InputBinds[identifier] =
			{
				CInputBindApi::IBToString(inputBind.Bind, true),
				inputBind
			};
			this->DisplayInputBinds.push_back(cat);
		}
		else
		{
			it->InputBinds[identifier] =
			{
				CInputBindApi::IBToString(inputBind.Bind, true),
				inputBind
			};
		}
	}
}

void CUiContext::UpdateDisplayGameBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	this->DisplayGameBinds.clear();

	CContext* ctx = CContext::GetContext();
	CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();

	/* copy of all InputBinds */
	std::unordered_map<EGameBinds, InputBind> InputBindRegistry = gameBindsApi->GetRegistry();

	/* acquire categories */
	for (auto& [identifier, inputBind] : InputBindRegistry)
	{
		std::string catName = CategoryNameFrom(identifier);

		auto it = std::find_if(this->DisplayGameBinds.begin(), this->DisplayGameBinds.end(), [catName](GameInputBindCategory category) { return category.Name == catName; });

		if (it == this->DisplayGameBinds.end())
		{
			GameInputBindCategory cat{};
			cat.Name = catName;
			cat.GameInputBinds[identifier] =
			{
				NameFrom(identifier),
				CInputBindApi::IBToString(inputBind, true),
				inputBind
			};
			this->DisplayGameBinds.push_back(cat);
		}
		else
		{
			it->GameInputBinds[identifier] =
			{
				NameFrom(identifier),
				CInputBindApi::IBToString(inputBind, true),
				inputBind
			};
		}
	}
}
