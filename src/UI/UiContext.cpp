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
#include <shellscalingapi.h>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

#include "Core/Addons/AddConst.h"
#include "Core/Addons/Addon.h"
#include "Core/Context.h"
#include "Core/Index/Index.h"
#include "Core/Preferences/PrefConst.h"
#include "Core/Preferences/PrefContext.h"
#include "Engine/Inputs/InputBinds/IbConst.h"
#include "GW2/Inputs/GameBinds/GbConst.h"
#include "GW2/Mumble/MblConst.h"
#include "GW2/Mumble/MblReader.h"
#include "res/ResConst.h"
#include "Util/Base64.h"
#include "Util/Inputs.h"
#include "Util/Resources.h"
#include "Util/Strings.h"
#include "Util/Time.h"

/*static*/ void CUiContext::OnInputBindPressed(const char* aIdentifier)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	if (aIdentifier == KB_TOGGLEHIDEUI)
	{
		uictx->IsVisible = !uictx->IsVisible;
	}
}

/*static*/ void CUiContext::OnFontUpdate(const char* aIdentifier, ImFont* aFont)
{
	std::string str = aIdentifier;

	if (str == "FONT_DEFAULT")
	{
		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = aFont; // TODO: This is kind of not needed? The Atlas[0] is used, if this is set to null.

		return;
	}

	CContext* ctx = CContext::GetContext();
	CDataLinkApi* dlapi = ctx->GetDataLink();

	Mumble::Identity* mumbleIdentity = static_cast<Mumble::Identity*>(dlapi->GetResource(DL_MUMBLE_LINK_IDENTITY));
	NexusLinkData_t* nexusLink = static_cast<NexusLinkData_t*>(dlapi->GetResource(DL_NEXUS_LINK));

	/* directly assign the font */
	switch (mumbleIdentity->UISize)
	{
		case Mumble::EUIScale::Small:
		{
			if (str == "MENOMONIA_S") { nexusLink->Font = aFont; }
			else if (str == "MENOMONIA_BIG_S") { nexusLink->FontBig = aFont; }
			else if (str == "FIRASANS_S") { nexusLink->FontUI = aFont; }

			break;
		}
		default:
		case Mumble::EUIScale::Normal:
		{
			if (str == "MENOMONIA_N") { nexusLink->Font = aFont; }
			else if (str == "MENOMONIA_BIG_N") { nexusLink->FontBig = aFont; }
			else if (str == "FIRASANS_N") { nexusLink->FontUI = aFont; }

			break;
		}
		case Mumble::EUIScale::Large:
		{
			if (str == "MENOMONIA_L") { nexusLink->Font = aFont; }
			else if (str == "MENOMONIA_BIG_L") { nexusLink->FontBig = aFont; }
			else if (str == "FIRASANS_L") { nexusLink->FontUI = aFont; }

			break;
		}
		case Mumble::EUIScale::Larger:
		{
			if (str == "MENOMONIA_XL") { nexusLink->Font = aFont; }
			else if (str == "MENOMONIA_BIG_XL") { nexusLink->FontBig = aFont; }
			else if (str == "FIRASANS_XL") { nexusLink->FontUI = aFont; }

			break;
		}
	}
}

/*static*/ void CUiContext::OnMumbleIdentityChanged(void* aEventArgs)
{
	CContext* ctx = CContext::GetContext();
	CDataLinkApi* dlapi = ctx->GetDataLink();
	CSettings* settingsctx = ctx->GetSettingsCtx();
	CUiContext* uictx = ctx->GetUIContext();
	CFontManager* fontmgr = uictx->GetFontManager();

	Mumble::Identity* mumbleIdentity = static_cast<Mumble::Identity*>(dlapi->GetResource(DL_MUMBLE_LINK_IDENTITY));
	NexusLinkData_t* nexusLink = static_cast<NexusLinkData_t*>(dlapi->GetResource(DL_NEXUS_LINK));

	switch (mumbleIdentity->UISize)
	{
		case Mumble::EUIScale::Small:
		{
			nexusLink->Font = fontmgr->Get("MENOMONIA_S")->Pointer;
			nexusLink->FontBig = fontmgr->Get("MENOMONIA_BIG_S")->Pointer;
			nexusLink->FontUI = fontmgr->Get("FIRASANS_S")->Pointer;
			break;
		}
		default:
		case Mumble::EUIScale::Normal:
		{
			nexusLink->Font = fontmgr->Get("MENOMONIA_N")->Pointer;
			nexusLink->FontBig = fontmgr->Get("MENOMONIA_BIG_N")->Pointer;
			nexusLink->FontUI = fontmgr->Get("FIRASANS_N")->Pointer;
			break;
		}
		case Mumble::EUIScale::Large:
		{
			nexusLink->Font = fontmgr->Get("MENOMONIA_L")->Pointer;
			nexusLink->FontBig = fontmgr->Get("MENOMONIA_BIG_L")->Pointer;
			nexusLink->FontUI = fontmgr->Get("FIRASANS_L")->Pointer;
			break;
		}
		case Mumble::EUIScale::Larger:
		{
			nexusLink->Font = fontmgr->Get("MENOMONIA_XL")->Pointer;
			nexusLink->FontBig = fontmgr->Get("MENOMONIA_BIG_XL")->Pointer;
			nexusLink->FontUI = fontmgr->Get("FIRASANS_XL")->Pointer;
			break;
		}
	}
}

/*static*/ void CUiContext::OnInputBindUpdate(void* aEventData)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	if (uictx)
	{
		uictx->Invalidate();
	}
}

CUiContext::CUiContext(
	RenderContext_t* aRenderContext,
	CLogApi*         aLogger,
	CTextureLoader*  aTextureService,
	CDataLinkApi*    aDataLink,
	CInputBindApi*   aInputBindApi,
	CEventApi*       aEventApi,
	CMumbleReader*   aMumbleReader
) : IRefCleaner("UiContext")
{
	this->RenderContext  = aRenderContext;
	this->Logger         = aLogger;
	this->TextureService = aTextureService;
	this->DataLink       = aDataLink;
	this->InputBindApi   = aInputBindApi;
	this->EventApi       = aEventApi;
	this->MumbleReader   = aMumbleReader;

	ImGui::CreateContext();

	this->Language       = new CLocalization(aLogger);
	this->Alerts         = new CAlerts(aDataLink);
	this->MainWindow     = new CMainWindow();
	this->QuickAccess    = new CQuickAccess(aDataLink, aLogger, aInputBindApi, aTextureService, this->Language, aEventApi);

	this->FontManager    = new CFontManager(this->Language);
	this->EscapeClose    = new CEscapeClosing();
	this->Scaling        = new CScaling(aRenderContext, aDataLink, aEventApi, CContext::GetContext()->GetSettingsCtx()); // FIXME: What the fuck, why is the settingsctx not included here?
	this->Input          = new CUiInput(CContext::GetContext()->GetSettingsCtx());

	this->EventApi->Subscribe(EV_MUMBLE_IDENTITY_UPDATED,              CUiContext::OnMumbleIdentityChanged);
	this->EventApi->Subscribe("EV_INPUTBIND_UPDATED",                  CUiContext::OnInputBindUpdate);

	this->InputBindApi->Register(KB_TOGGLEHIDEUI, EIbHandlerType::DownAsync, CUiContext::OnInputBindPressed, "CTRL+H");
	this->EscapeClose->Register("Nexus", this->MainWindow->GetVisibleStatePtr());
	
	this->ApplyStyle();
	this->LoadFonts();
}

CUiContext::~CUiContext()
{
	ImGui::DestroyContext();
}

void CUiContext::Initialize()
{
	if (this->IsInitialized)
	{
		return;
	}

	if (!(this->RenderContext->Window.Handle &&
		this->RenderContext->Device &&
		this->RenderContext->DeviceContext &&
		this->RenderContext->SwapChain))
	{
		this->Logger->Critical(CH_UICONTEXT, "CUiContext::Initialize() failed. A RenderContext component was nullptr.");
		return;
	}

	// Init imgui
	ImGui_ImplWin32_Init(this->RenderContext->Window.Handle);
	ImGui_ImplDX11_Init(this->RenderContext->Device, this->RenderContext->DeviceContext);
	//ImGui::GetIO().ImeWindowHandle = Renderer::WindowHandle;

	// create buffers
	ID3D11Texture2D* pBackBuffer{};
	this->RenderContext->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

	if (!pBackBuffer)
	{
		this->Logger->Critical(CH_UICONTEXT, "CUiContext::Initialize() failed. BackBuffer was nullptr.");
		return;
	}

	this->RenderContext->Device->CreateRenderTargetView(pBackBuffer, NULL, &this->RenderTargetView);
	pBackBuffer->Release();

	if (!this->RenderTargetView)
	{
		this->Logger->Critical(CH_UICONTEXT, "CUiContext::Initialize() failed. RenderTargetView could not be created.");
		return;
	}

	this->Scaling->UpdateDPI(); // Update DPI, because the HWND is now available.

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

void CUiContext::Render()
{
	this->Initialize();

	/* preload localization and font changes */
	if (this->Language->Advance())
	{
		this->FontManager->Reload();
		this->Invalidate();
	}
	if (this->FontManager->Advance())
	{
		CUiContext::OnMumbleIdentityChanged(nullptr);
		this->Shutdown();
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

	const std::lock_guard<std::mutex> lock(this->RenderMutex);

	/* pre-render callbacks */
	for (GUI_RENDER callback : this->GetRenderCallbacks(ERenderType::PreRender))
	{
		callback();
	}

	if (this->IsInitialized)
	{
		/* new frame */
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		static CContext*  s_Context      = CContext::GetContext();
		static CSettings* s_Settings     = s_Context->GetSettingsCtx();
		static bool       s_EulaAccepted = s_Settings->Get<bool>(OPT_ACCEPTEULA, false);
		
		if (s_EulaAccepted)
		{
			/* normal overlay */
			if (this->IsVisible)
			{
				/* draw addons*/
				for (GUI_RENDER callback : this->GetRenderCallbacks(ERenderType::Render))
				{
					callback();
				}

				/* draw nexus windows */
				this->Alerts->Render();
				this->MainWindow->Render();
				this->QuickAccess->Render();
			}
		}
		else
		{
			/* license agreement */
			static CLicenseAgreementModal s_Modal = {};
			static bool s_Opened = [] {
				s_Modal.OpenModal();
				return true;
			}();
			
			if (s_Modal.Render())
			{
				/* Update state. */
				s_EulaAccepted = s_Settings->Get<bool>(OPT_ACCEPTEULA, false);

				if (s_EulaAccepted)
				{
					/* activate main window for the first time */
					this->MainWindow->Activate();
				}
				else
				{
					PostMessageA(this->RenderContext->Window.Handle, WM_CLOSE, 0, 0);
				}
			}
		}

		/* end frame */
		ImGui::EndFrame();
		ImGui::Render();
		this->RenderContext->DeviceContext->OMSetRenderTargets(1, &this->RenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	/* post-render callbacks */
	for (GUI_RENDER callback : this->GetRenderCallbacks(ERenderType::PostRender))
	{
		callback();
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

	if (this->Input->WndProc(hWnd, uMsg, wParam, lParam) == 0)
	{
		return 0;
	}

	if (this->EscapeClose->WndProc(hWnd, uMsg, wParam, lParam) == 0)
	{
		return 0;
	}

	return uMsg;
}

CLocalization* CUiContext::GetLocalization()
{
	return this->Language;
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

void CUiContext::LoadFonts()
{
	std::filesystem::path fontPath{};

	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	/* add user font */
	bool hasUserFont = false;
	float storedFontSz = settingsctx->Get<float>(OPT_FONTSIZE, 15.0f);
	storedFontSz = min(max(storedFontSz, 1.0f), 50.0f);

	std::string fontFile = settingsctx->Get<std::string>(OPT_USERFONT, "");
	if (!fontFile.empty() && std::filesystem::exists(Index(EPath::DIR_FONTS) / fontFile))
	{
		fontPath = Index(EPath::DIR_FONTS) / fontFile;
		this->FontManager->ReplaceFont("FONT_DEFAULT", storedFontSz, fontPath.string().c_str(), CUiContext::OnFontUpdate, nullptr);
		hasUserFont = true;
	}

	/* add default font for monospace */
	this->FontManager->AddDefaultFont(CUiContext::OnFontUpdate);

	if (!hasUserFont)
	{
		this->FontManager->ResizeFont("FONT_DEFAULT", storedFontSz);
	}

	ImFontConfig config;
	config.MergeMode = true;

	/* small UI*/
	this->FontManager->ReplaceFont("MENOMONIA_S", 16.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_S_MERGE", 16.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_S", 22.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_S_MERGE", 22.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("FIRASANS_S", 15.0f, RES_FONT_FIRASANS, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("FIRASANS_S_MERGE", 15.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }

	/* normal UI*/
	this->FontManager->ReplaceFont("MENOMONIA_N", 18.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_N_MERGE", 18.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_N", 24.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_N_MERGE", 24.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("FIRASANS_N", 16.0f, RES_FONT_FIRASANS, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("FIRASANS_N_MERGE", 16.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }

	/* large UI*/
	this->FontManager->ReplaceFont("MENOMONIA_L", 20.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_L_MERGE", 20.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_L", 26.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_L_MERGE", 26.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("FIRASANS_L", 17.5f, RES_FONT_FIRASANS, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("FIRASANS_L_MERGE", 17.5f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }

	/* larger UI*/
	this->FontManager->ReplaceFont("MENOMONIA_XL", 22.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_XL_MERGE", 22.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG_XL", 28.0f, RES_FONT_MENOMONIA, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_XL_MERGE", 28.0f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("FIRASANS_XL", 19.5f, RES_FONT_FIRASANS, ctx->GetModule(), CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("FIRASANS_XL_MERGE", 19.5f, fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
}
