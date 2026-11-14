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

#include "Host/GW2/Addons/AddConst.h"
#include "Host/GW2/Addons/Addon.h"
#include "Host/GW2/Context.h"
#include "Host/GW2/Index/Index.h"
#include "Engine/Inputs/InputBinds/IbConst.h"
#include "Host/GW2/Inputs/GameBinds/GbConst.h"
#include "Host/GW2/Mumble/MblConst.h"
#include "Host/GW2/Mumble/MblReader.h"
#include "res/Resources.h"
#include "Util/Base64.h"
#include "Util/Inputs.h"
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
	NexusLinkData_t* nexusLink = static_cast<NexusLinkData_t*>(ctx->GetDataLink()->GetResource(DL_NEXUS_LINK));

	if (!nexusLink) { return; }

	if (str == "MENOMONIA") { nexusLink->Font = aFont; }
	else if (str == "MENOMONIA_BIG") { nexusLink->FontBig = aFont; }
	else if (str == "FIRASANS") { nexusLink->FontUI = aFont; }
}

/*static*/ void CUiContext::OnMumbleIdentityChanged(void* aEventArgs)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	uictx->ApplyFontScale();
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
	CTextureLoader*  aTextureService,
	CDataLinkApi*    aDataLink,
	CInputBindApi*   aInputBindApi,
	CEventApi*       aEventApi,
	CMumbleReader*   aMumbleReader
) : IRefCleaner("UiContext"), ILogChannel("UI Context")
{
	this->RenderContext  = aRenderContext;
	this->TextureService = aTextureService;
	this->DataLink       = aDataLink;
	this->InputBindApi   = aInputBindApi;
	this->EventApi       = aEventApi;
	this->MumbleReader   = aMumbleReader;

	ImGui::CreateContext();

	this->Language       = new CLocalization();
	this->Alerts         = new CAlerts(aDataLink);
	this->MainWindow     = new CMainWindow();
	this->QuickAccess    = new CQuickAccess(aDataLink, aInputBindApi, aTextureService, this->Language, aEventApi);

	this->FontManager    = new CFontManager();
	this->EscapeClose    = new CEscapeClosing();
	this->Scaling        = new CScaling(aRenderContext, aDataLink, aEventApi);
	this->Input          = new CUiInput();

	this->EventApi->Subscribe(EV_MUMBLE_IDENTITY_UPDATED,              CUiContext::OnMumbleIdentityChanged);
	this->EventApi->Subscribe("EV_INPUTBIND_UPDATED",                  CUiContext::OnInputBindUpdate);

	this->InputBindApi->Register(KB_TOGGLEHIDEUI, EIbHandlerType::DownAsync, CUiContext::OnInputBindPressed, "CTRL+H");
	this->EscapeClose->Register("Nexus", this->MainWindow->GetVisibleStatePtr());
	
	this->ApplyStyle();
	this->LoadFonts();
	this->INotifyHandler<IWndProcNotifyHandler>::Attach(WNDPROC_PRIO_UI);
	this->INotifyHandler<IDxgiPresentNotifyHandler>::Attach(DXGIPRESENT_PRIO_UI);
	this->INotifyHandler<IDxgiResizeNotifyHandler>::Attach(DXGIRESIZE_PRIO_UI);
}

CUiContext::~CUiContext()
{
	this->INotifyHandler<IDxgiResizeNotifyHandler>::Detach();
	this->INotifyHandler<IDxgiPresentNotifyHandler>::Detach();
	this->INotifyHandler<IWndProcNotifyHandler>::Detach();

	ImGui::DestroyContext();
}

void CUiContext::Initialize()
{
	if (this->IsInitialized)
	{
		return;
	}

	if (!(this->RenderContext->Window.Handle && this->RenderContext->SwapChain))
	{
		this->LogCritical("CUiContext::Initialize() failed. A RenderContext component was nullptr.");
		return;
	}

	/* Retrieve BackBuffer. */
	ID3D11Texture2D* pBackBuffer{};
	this->RenderContext->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

	if (!pBackBuffer)
	{
		this->LogCritical("CUiContext::Initialize() failed. BackBuffer was nullptr.");
		return;
	}

	ID3D11Device* bbDevice = nullptr;
	pBackBuffer->GetDevice(&bbDevice);

	/* Retrieve Device and DeviceContext from BackBuffer. */
	if (this->RenderContext->Device == nullptr)
	{
		this->RenderContext->Device = bbDevice;
		this->RenderContext->Device->GetImmediateContext(&this->RenderContext->DeviceContext);
	}

	HRESULT hr = bbDevice->CreateRenderTargetView(pBackBuffer, NULL, &this->RenderTargetView);
	pBackBuffer->Release();

	if (!this->RenderTargetView)
	{
		this->LogCritical("CUiContext::Initialize() failed. RenderTargetView could not be created.");
		return;
	}

	this->Scaling->UpdateDPI(); // Update DPI, because the HWND is now available.

	// Init imgui
	ImGui_ImplWin32_Init(this->RenderContext->Window.Handle);
	ImGui_ImplDX11_Init(this->RenderContext->Device, this->RenderContext->DeviceContext);
	this->FontManager->CreateDeviceObjects(this->RenderContext->Device);
	//ImGui::GetIO().ImeWindowHandle = Renderer::WindowHandle;

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
		this->FontManager->SetTexts(this->Language->GetActiveTexts());
		this->Invalidate();
	}
	EFontsUpdate fontsUpdate = this->FontManager->Advance();
	if (fontsUpdate != EFontsUpdate::None)
	{
		if (fontsUpdate == EFontsUpdate::DefaultAtlas)
		{
			this->Shutdown();
		}
		else if (this->IsInitialized)
		{
			this->FontManager->CreateDeviceObjects(this->RenderContext->Device);
		}
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

	if (this->IsInitialized && this->FontManager->IsReady())
	{
		this->Input->FlushInput();

		/* new frame */
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		if (this->EulaAccepted.Get())
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
				if (this->EulaAccepted.Get())
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
		this->FontManager->CollectMissingGlyphs(ImGui::GetDrawData());
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

void CUiContext::OnDxgiPresentNotify(IDXGISwapChain* aSwapChain)
{
	this->Render();
}

void CUiContext::OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight)
{
	this->Shutdown();
}

UINT CUiContext::OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam)
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

CMainWindow* CUiContext::GetMainWindow()
{
	return this->MainWindow;
}

CScaling* CUiContext::GetScaling()
{
	return this->Scaling;
}

CUiInput* CUiContext::GetInput()
{
	return this->Input;
}

CEscapeClosing* CUiContext::GetEscapeClosingService()
{
	return this->EscapeClose;
}

/* Per UI scale: Font, FontBig, FontUI. The user font merged into each has the same size. */
static constexpr float s_ScaleFontSizes[4][3] =
{
	{ 16.0f, 22.0f, 15.0f }, /* Small */
	{ 18.0f, 24.0f, 16.0f }, /* Normal */
	{ 20.0f, 26.0f, 17.5f }, /* Large */
	{ 22.0f, 28.0f, 19.5f }  /* Larger */
};

static size_t GetScaleIndex(Mumble::Identity* aIdentity)
{
	if (!aIdentity) { return 1; }

	switch (aIdentity->UISize)
	{
		case Mumble::EUIScale::Small:  { return 0; }
		case Mumble::EUIScale::Large:  { return 2; }
		case Mumble::EUIScale::Larger: { return 3; }
		default:                       { return 1; }
	}
}

void CUiContext::ApplyFontScale()
{
	const float* sizes = s_ScaleFontSizes[GetScaleIndex(static_cast<Mumble::Identity*>(this->DataLink->GetResource(DL_MUMBLE_LINK_IDENTITY)))];

	this->FontManager->ResizeFont("MENOMONIA",           sizes[0]);
	this->FontManager->ResizeFont("MENOMONIA_MERGE",     sizes[0]);
	this->FontManager->ResizeFont("MENOMONIA_BIG",       sizes[1]);
	this->FontManager->ResizeFont("MENOMONIA_BIG_MERGE", sizes[1]);
	this->FontManager->ResizeFont("FIRASANS",            sizes[2]);
	this->FontManager->ResizeFont("FIRASANS_MERGE",      sizes[2]);
}

void CUiContext::LoadFonts()
{
	std::filesystem::path fontPath{};

	this->FontManager->SetGlyphCache(Index(EPath::GlyphCache));

	/* None of the embedded fonts has CJK glyphs. System fonts are not redistributed and add nothing to the binary. */
	wchar_t windowsDir[MAX_PATH]{};
	if (GetWindowsDirectoryW(windowsDir, MAX_PATH))
	{
		std::filesystem::path systemFonts = std::filesystem::path(windowsDir) / "Fonts";
		this->FontManager->AddFallbackFont(systemFonts / "msyh.ttc");   /* Microsoft YaHei: Chinese, kana */
		this->FontManager->AddFallbackFont(systemFonts / "malgun.ttf"); /* Malgun Gothic: Hangul */
	}

	/* add user font */
	bool hasUserFont = false;
	float storedFontSz = this->FontManager->FontSize.Get();
	storedFontSz = min(max(storedFontSz, 1.0f), 50.0f);

	std::string fontFile = this->FontManager->UserFont.Get();
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

	const float* sizes = s_ScaleFontSizes[GetScaleIndex(static_cast<Mumble::Identity*>(this->DataLink->GetResource(DL_MUMBLE_LINK_IDENTITY)))];

	this->FontManager->ReplaceFont("MENOMONIA", sizes[0], Resources::FontMenomonia, CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_MERGE", sizes[0], fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("MENOMONIA_BIG", sizes[1], Resources::FontMenomonia, CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("MENOMONIA_BIG_MERGE", sizes[1], fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
	this->FontManager->ReplaceFont("FIRASANS", sizes[2], Resources::FontFiraSans, CUiContext::OnFontUpdate, nullptr);
	if (!fontPath.empty()) { this->FontManager->ReplaceFont("FIRASANS_MERGE", sizes[2], fontPath.string().c_str(), CUiContext::OnFontUpdate, &config); }
}

