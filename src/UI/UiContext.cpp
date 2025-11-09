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
#include "Resources/ResConst.h"
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

/*static*/ void CUiContext::OnUELanguageChanged(uint32_t* aLanguage)
{
	if (!aLanguage) { return; }

	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();
	CLocalization* localization = uictx->GetLocalization();

	switch (*aLanguage)
	{
		case 0:
		{
			localization->SetLanguage("en");
			break;
		}
		case 2:
		{
			localization->SetLanguage("fr");
			break;
		}
		case 3:
		{
			localization->SetLanguage("de");
			break;
		}
		case 4:
		{
			localization->SetLanguage("es");
			break;
		}
		case 5:
		{
			localization->SetLanguage("cn");
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

/*static*/ void CUiContext::OnVolatileAddonsDisabled(void* aEventData)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	if (settingsctx->Get<bool>(OPT_SHOWADDONSWINDOWAFTERDUU, false))
	{
		uictx->MainWindow->Activate();
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
	this->EventApi->Subscribe("EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED", reinterpret_cast<EVENT_CONSUME>(CUiContext::OnUELanguageChanged));
	this->EventApi->Subscribe(EV_VOLATILE_ADDON_DISABLED,              CUiContext::OnVolatileAddonsDisabled);
	this->EventApi->Subscribe("EV_INPUTBIND_UPDATED",                  CUiContext::OnInputBindUpdate);

	this->InputBindApi->Register(KB_TOGGLEHIDEUI, EIbHandlerType::DownAsync, CUiContext::OnInputBindPressed, "CTRL+H");
	this->EscapeClose->Register("Nexus", this->MainWindow->GetVisibleStatePtr());

	this->UnpackLocales();
	this->LoadSettings();
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

void ApplyDefaultStyle()
{
	try
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		static std::string s_NexusStyleDefault = "AACAPwAAAEEAAABBAAAAAAAAgD8AAABCAAAAQgAAAAAAAAA/AAAAAAAAAAAAAIA/AAAAAAAAgD8AAIBAAABAQAAAAAAAAAAAAAAAQQAAgEAAAIBAAACAQAAAgEAAAABAAAAAAAAAAAAAAKhBAADAQAAAYEEAABBBAAAgQQAAAAAAAIBAAACAQAAAAAAAAAAAAQAAAAAAAD8AAAA/AAAAAAAAAAAAAJhBAACYQQAAQEAAAEBAAACAPwEBAQAAAKA/zczMPwAAgD8AAIA/AACAPwAAgD8AAAA/AAAAPwAAAD8AAIA/j8J1PY/CdT2PwnU916NwPwAAAAAAAAAAAAAAAAAAAAAK16M9CtejPQrXoz3Xo3A/9ijcPvYo3D4AAAA/AAAAPwAAAAAAAAAAAAAAAAAAAAAK1yM+4XqUPo/C9T5xPQo/uB6FPj0KFz9I4Xo/zczMPrgehT49Chc/SOF6Px+FKz8K1yM9CtcjPQrXIz0AAIA/CtcjPuF6lD6PwvU+AACAPwAAAAAAAAAAAAAAAFyPAj8pXA8+KVwPPilcDz4AAIA/CtejPArXozwK16M8FK4HP1K4nj5SuJ4+UriePgAAgD+F69E+hevRPoXr0T4AAIA/XI8CP1yPAj9cjwI/AACAP7gehT49Chc/SOF6PwAAgD+PwnU+uB4FP65HYT8AAIA/uB6FPj0KFz9I4Xo/AACAP7gehT49Chc/SOF6P83MzD64HoU+PQoXP0jhej8AAIA/j8J1PRSuBz9I4Xo/AACAP7gehT49Chc/SOF6P1K4nj64HoU+PQoXP0jhej/NzEw/uB6FPj0KFz9I4Xo/AACAP/Yo3D72KNw+AAAAPwAAAD/NzMw9zczMPgAAQD8Urkc/zczMPc3MzD4AAEA/AACAP7gehT49Chc/SOF6P83MTD64HoU+PQoXP0jhej8fhSs/uB6FPj0KFz9I4Xo/MzNzP+tROD4yM7M+4noUPwisXD+4HoU+PQoXP0jhej/NzEw/zMxMPoTr0T57FC4/AACAP5ZDiz1e5dA9Uo0XPjXveD+UQws+3CSGPocW2T4AAIA/9igcP/YoHD/2KBw/AACAPwAAgD/2KNw+MzOzPgAAgD9mZmY/MzMzPwAAAAAAAIA/AACAP5qZGT8AAAAAAACAP1yPQj5cj0I+zcxMPgAAgD9SuJ4+UriePjMzsz4AAIA/H4VrPh+Faz4AAIA+AACAPwAAAAAAAAAAAAAAAAAAAAAAAIA/AACAPwAAgD+PwnU9uB6FPj0KFz9I4Xo/MzOzPgAAgD8AAIA/AAAAAGZmZj+4HoU+PQoXP0jhej8AAIA/AACAPwAAgD8AAIA/MzMzP83MTD/NzEw/zcxMP83MTD7NzEw/zcxMP83MTD8zM7M+";
		std::string decodeStyle = Base64::Decode(s_NexusStyleDefault, s_NexusStyleDefault.length());
		memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
	}
	catch (...)
	{
		CContext::GetContext()->GetLogger()->Debug(CH_UICONTEXT, "Error applying default style.");
	}
}

void CUiContext::ApplyStyle(EUIStyle aStyle, std::string aValue)
{
	ImGuiStyle* style = &ImGui::GetStyle();

	switch (aStyle)
	{
		case EUIStyle::User:
		{
			try
			{
				CContext* ctx = CContext::GetContext();
				CSettings* settingsctx = ctx->GetSettingsCtx();

				std::string b64_style = settingsctx->Get<std::string>(OPT_IMGUISTYLE, {});

				if (b64_style.empty())
				{
					this->ApplyStyle(EUIStyle::Nexus);
					return;
				}
				std::string decodeStyle = Base64::Decode(b64_style, b64_style.length());
				memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying user style.");
			}
			return;
		}
		case EUIStyle::Nexus:
		{
			try
			{
				CContext* ctx = CContext::GetContext();
				CSettings* settingsctx = ctx->GetSettingsCtx();

				std::string b64_style = "AACAPwAAoEAAAKBAAADAQAAAgD8AAMBAAACAQAAAAAAAAAA/AAAAAAAAwEAAAIA/AADAQAAAgD8AAMBAAAAAQAAAwEAAAAAAAADAQAAAgEAAAMBAAACAQAAAwEAAAIBAAAAAAAAAAAAAAABCAADAQAAAQEEAAMBAAAAAQgAAwEAAAIBAAADAQAAAAAAAAAAAAQAAAAAAAD8AAAA/AAAAAAAAAAAAAJhBAACYQQAAQEAAAEBAAACAPwEBAQAAAKA/zczMPwAAgD8AAIA/AACAPwAAgD+rqio/q6oqP6uqKj8AAIA/mZiYPbGwsD3h4OA97+5uP5mYmD2xsLA94eDgPYmICD+ZmJg9sbCwPeHg4D3e3V0/gYAAP4GAAD+BgAA/mpkZPwrXIz9SuB4/H4UrPwAAAABSuB4/mpkZP2ZmJj/NzEw+UrgeP5qZGT9mZiY/AABAPylcDz8pXA8/4XoUPwAAQD+ZmJg9sbCwPeHg4D0AAIA/mZiYPbGwsD3h4OA9AACAP5mYmD2xsLA94eDgPQAAgD+ZmJg9sbCwPeHg4D0AAIA/mZiYPbGwsD3h4OA9AACAPx+F6z5mZuY+16PwPhSuRz8fhSs/H4UrP9ejMD8Urkc/FK5HPxSuRz/NzEw/FK5HP83MTD/NzEw/4XpUPylcTz/NzEw/zcxMP+F6VD9SuJ4+j8J1Pc3MTD0pXI89AACAP1K4Hj+amRk/ZmYmP5qZmT5SuB4/mpkZP2ZmJj+amRk/UrgeP5qZGT9mZiY/ZmZmP+xRuD7sUbg+XI/CPjMzMz/sUbg+7FG4PlyPwj4zM7M+7FG4PuxRuD5cj8I+MzMzPwAAAD8AAAA/AAAAP5qZGT+amRk/mpkZPzMzMz8AAIA/MzMzPzMzMz9mZmY/AACAPwAAAAAAAAAAAAAAAAAAAAApXA8/KVwPP+F6FD8AAIA/j8J1Pc3MTD0pXI89AACAPzMzMz97FC4/16MwP83MzD0zMzM/exQuP9ejMD+amZk+MzMzP3sULj/XozA/9ijcPpf/kD6X/5A+4ZwRPyo6Uj+hZ7M+oWezPkLPJj+9UlY/MzMzP3sULj/D9Sg/KVwPPwAAgD4AAIA/AAAAAAAAgD8zMzM/exQuP8P1KD+PwvU+AACAPgAAgD8AAAAAAACAP3E9ij5xPYo+XI/CPgAAgD9SuJ4+UriePmZm5j4AAIA/uB6FPrgehT4pXI8+AACAPwAAAAAAAAAAAAAAAAAAAAAAAIA/AACAPwAAgD8pXI897FG4PuxRuD6uR2E/zcwMPwAAgD8AAIA/AAAAAGZmZj9mZuY+ZmbmPmZmZj/NzEw/AACAPwAAgD8AAIA/MzMzP83MTD/NzEw/zcxMP83MTD7NzEw+zcxMPs3MTD4zM7M+";
				std::string decodeStyle = Base64::Decode(b64_style, b64_style.length());
				memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying user style.");
			}
			return;
		}
		case EUIStyle::ImGui_Classic:
		{
			
			ApplyDefaultStyle();
			ImGui::StyleColorsClassic();
			return;
		}
		case EUIStyle::ImGui_Light:
		{
			ApplyDefaultStyle();
			ImGui::StyleColorsLight();
			return;
		}
		case EUIStyle::ImGui_Dark:
		{
			ApplyDefaultStyle();
			ImGui::StyleColorsDark();
			return;
		}
		case EUIStyle::ArcDPS_Default:
		{
			try
			{
				static std::string s_ArcStyleDefault = "AACAPwAAgEAAAIBAAAAAAAAAAAAAAKBAAABAQAAAAAAAAAA/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAIBAAACAQAAAAAAAAAAAAACgQAAAQEAAAKBAAABAQAAAQEAAAAAAAAAAAAAAAAAAAMhBAADAQAAAEEEAAAAAAADIQQAAAAAAAIBAAAAAAAAAAAAAAAAAAQAAAAAAAD8AAAA/AAAAAAAAAAAAAJhBAACYQQAAQEAAAEBAAACAPwEBAQAAAKA/zczMPw==";
				std::string decodeStyle = Base64::Decode(s_ArcStyleDefault, s_ArcStyleDefault.length());
				memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());

				static std::string s_ArcColorsDefault = "zcxMP83MTD/helQ/AACAP4/CdT4fhWs+4XqUPgAAgD+PwnU9zcxMPSlcjz0AAEA/KVyPPSlcjz3sUbg9AAAAAClcjz0pXI897FG4PZqZWT8K1yM/hesRPzMzMz/NzEw+CtcjP1K4Hj8fhSs/AAAAAFK4Hj+amRk/ZmYmP83MTD5SuB4/mpkZP2ZmJj8AAEA/KVwPPylcDz/hehQ/AABAP83MzD3sUbg9j8L1PZqZWT/NzMw97FG4PY/C9T2amVk/zczMPexRuD2PwvU9mplZP83MzD3sUbg9j8L1PTMzMz/NzMw97FG4PY/C9T2amRk/H4XrPmZm5j7Xo/A+FK5HPx+FKz8fhSs/16MwPxSuRz8Urkc/FK5HP83MTD8Urkc/zcxMP83MTD/helQ/KVxPP83MTD/NzEw/4XpUP1K4nj6PwnU9zcxMPSlcjz0AAIA/UrgeP5qZGT9mZiY/mpmZPlK4Hj+amRk/ZmYmP5qZGT9SuB4/mpkZP2ZmJj9mZmY/7FG4PuxRuD5cj8I+MzMzP+xRuD7sUbg+XI/CPjMzsz7sUbg+7FG4PlyPwj4zMzM/AAAAPwAAAD8AAAA/mpkZP5qZGT+amRk/MzMzPwAAgD8zMzM/MzMzP2ZmZj8AAIA/AAAAAAAAAAAAAAAAAAAAAClcDz8pXA8/4XoUPwAAgD+PwnU9zcxMPSlcjz0AAIA/MzMzP3sULj/XozA/zczMPTMzMz97FC4/16MwP5qZmT4zMzM/exQuP9ejMD/2KNw+l/+QPpf/kD7hnBE/KjpSP6Fnsz6hZ7M+Qs8mP71SVj8zMzM/exQuP8P1KD8pXA8/AACAPgAAgD8AAAAAAACAPzMzMz97FC4/w/UoP4/C9T4AAIA+AACAPwAAAAAAAIA/cT2KPnE9ij5cj8I+AACAP1K4nj5SuJ4+ZmbmPgAAgD+4HoU+uB6FPilcjz4AAIA/AAAAAAAAAAAAAAAAAAAAAAAAgD8AAIA/AACAPylcjz3sUbg+7FG4Pq5HYT/NzAw/AACAPwAAgD8AAAAAZmZmP2Zm5j5mZuY+ZmZmP83MTD8AAIA/AACAPwAAgD8zMzM/zcxMP83MTD/NzEw/zcxMPs3MTD7NzEw+zcxMPjMzsz4=";
				std::string decodeColors = Base64::Decode(s_ArcColorsDefault, s_ArcColorsDefault.length());
				memcpy_s(&style->Colors[0], sizeof(ImVec4) * ImGuiCol_COUNT, &decodeColors[0], decodeColors.length());
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying ArcDPS default style.");
			}
			return;
		}
		case EUIStyle::ArcDPS_Current:
		{
			std::filesystem::path arcIniPath = Index(EPath::DIR_ADDONS) / "arcdps/arcdps.ini";

			if (std::filesystem::exists(arcIniPath))
			{
				char buff[4096]{};
				std::string decode{};

				try
				{
					memset(buff, 0, sizeof(buff)); // reset buffer
					GetPrivateProfileStringA("session", "appearance_imgui_style180", "", &buff[0], sizeof(buff), arcIniPath.string().c_str());
					std::string arcstyle = buff;
					decode = Base64::Decode(arcstyle, arcstyle.length());
					memcpy(style, &decode[0], decode.length());

					memset(buff, 0, sizeof(buff)); // reset buffer
					GetPrivateProfileStringA("session", "appearance_imgui_colours180", "", &buff[0], sizeof(buff), arcIniPath.string().c_str());
					std::string arccols = buff;
					decode = Base64::Decode(arccols, arccols.length());
					memcpy(&style->Colors[0], &decode[0], decode.length());
				}
				catch (...)
				{
					this->Logger->Warning(CH_UICONTEXT, "Couldn't parse ArcDPS style.");
				}
			}
			else
			{
				this->Logger->Warning(CH_UICONTEXT, "Tried importing ArcDPS style, with no config present.");
			}
			return;
		}
		case EUIStyle::File:
		{
			std::filesystem::path path = Index(EPath::DIR_STYLES) / aValue;

			if (std::filesystem::is_directory(path)) { return; }
			if (std::filesystem::file_size(path) == 0) { return; }
			if (path.extension() != ".imstyle180") { return; }

			ImGuiStyle* style = &ImGui::GetStyle();

			try
			{
				CContext* ctx = CContext::GetContext();
				CSettings* settingsctx = ctx->GetSettingsCtx();

				std::ifstream file(path);

				if (file)
				{
					std::string b64_style;
					std::getline(file, b64_style);
					file.close();

					std::string decodeStyle = Base64::Decode(b64_style, b64_style.length());

					if (decodeStyle.size() != sizeof(ImGuiStyle))
					{
						this->Logger->Warning(CH_UICONTEXT, "Error applying stylesheet. Not ImGui 1.80 compatible.");
						return;
					}

					memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
				}
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying stylesheet.");
			}

			break;
		}
		case EUIStyle::Code:
		{
			std::string decodeStyle = Base64::Decode(aValue, aValue.length());

			if (decodeStyle.size() != sizeof(ImGuiStyle))
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying stylesheet. Not ImGui 1.80 compatible.");
				return;
			}

			memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
			break;
		}
	}
}

void CUiContext::UnpackLocales()
{
	CContext* ctx = CContext::GetContext();

	this->Language->SetLocaleDirectory(Index(EPath::DIR_LOCALES));
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleEN), RES_LOCALE_EN, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleDE), RES_LOCALE_DE, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleFR), RES_LOCALE_FR, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleES), RES_LOCALE_ES, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleBR), RES_LOCALE_BR, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleCZ), RES_LOCALE_CZ, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleIT), RES_LOCALE_IT, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocalePL), RES_LOCALE_PL, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleRU), RES_LOCALE_RU, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleCN), RES_LOCALE_CN, "JSON");
	this->Language->Advance();
}

void CUiContext::LoadSettings()
{
	CContext* ctx = CContext::GetContext();
	CSettings* settingsCtx = ctx->GetSettingsCtx();

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle* style = &ImGui::GetStyle();

	float storedFontSz = settingsCtx->Get<float>(OPT_FONTSIZE, 15.0f);
	if (storedFontSz <= 0)
	{
		storedFontSz = min(max(storedFontSz, 1.0f), 50.0f);
		settingsCtx->Set(OPT_FONTSIZE, storedFontSz);
	}
	ImGui::GetCurrentContext()->FontSize = storedFontSz;

	this->ApplyStyle();
	
	std::string lang = settingsCtx->Get<std::string>(OPT_LANGUAGE, "en");
	Language->SetLanguage(!lang.empty() ? lang : "en");
}
