#include "Main.h"

#include <filesystem>
#include <Psapi.h>
#include <string>
#include <windowsx.h>

#include "Consts.h"
#include "Engine/Index/Index.h"
#include "resource.h"
#include "Shared.h"
#include "State.h"

#include "Context.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/API/ApiClient.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Logging/LogConsole.h"
#include "Engine/Logging/LogWriter.h"
#include "Engine/Logging/LogApi.h"
#include "GW2/Multibox/Multibox.h"
#include "GW2/Mumble/Reader.h"
#include "Engine/Settings/Settings.h"
#include "Engine/Updater/Updater.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"
#include "Util/Resources.h"
#include "Util/Inputs.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

/* entry */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			CContext* ctx = CContext::GetContext();
			ctx->SetModule(hModule);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			CContext* ctx = CContext::GetContext();

			/* cleanly remove wndproc hook */
			if (ctx->GetRendererCtx()->Window.Handle && Hooks::WndProc)
			{
				SetWindowLongPtr(ctx->GetRendererCtx()->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Hooks::WndProc);
			}
			break;
		}
	}
	return true;
}

namespace Hooks
{
	DXPRESENT       DXGIPresent       = nullptr;
	DXRESIZEBUFFERS DXGIResizeBuffers = nullptr;
	WNDPROC         WndProc           = nullptr;
}

namespace Main
{
	static NexusLinkData_t*  s_NexusLink      = nullptr;
	static CRawInputApi*     s_RawInputApi    = nullptr;
	static CInputBindApi*    s_InputBindApi   = nullptr;
	static CUiContext*       s_UIContext      = nullptr;
	static CTextureLoader*   s_TextureService = nullptr;
	static CEventApi*        s_EventApi       = nullptr;
	static CSettings*        s_SettingsCtx    = nullptr;
	static RenderContext_t*  s_RendererCtx    = nullptr;

	void Initialize(EEntryMethod aEntryMethod)
	{
		if (State::Nexus >= ENexusState::LOAD) { return; }

		State::Nexus = ENexusState::LOAD;

		CContext* ctx = CContext::GetContext();
		s_RendererCtx = ctx->GetRendererCtx();

		//SetUnhandledExceptionFilter(UnhandledExcHandler);
		
		CreateIndex(ctx->GetModule());

		Resources::Unpack(ctx->GetModule(), Index(EPath::ThirdPartySoftwareReadme), RES_THIRDPARTYNOTICES, "TXT");

		CLogApi* logger = ctx->GetLogger();
		CUpdater* updater = ctx->GetUpdater();

		/* setup default loggers */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			logger->Register(new CConsoleLogger(ELogLevel::ALL));
		}

		std::filesystem::path logPathOverride;

		if (CmdLine::HasArgument("-mumble"))
		{
			logPathOverride = (Index(EPath::DIR_NEXUS) / "Nexus_").string() + CmdLine::GetArgumentValue("-mumble") + ".log";
		}

		logger->Register(new CFileLogger(ELogLevel::ALL, logPathOverride.empty() ? Index(EPath::Log) : logPathOverride));

		logger->Info(CH_CORE, GetCommandLineA());
		logger->Info(CH_CORE, "%s: %s", Index(EPath::NexusDLL) != Index(EPath::D3D11Chainload) ? "Proxy" : "Chainload", Index(EPath::NexusDLL).string().c_str());
		logger->Info(CH_CORE, "Nexus: %s %s", ctx->GetVersion().string().c_str(), ctx->GetBuild());
		logger->Info(CH_CORE, "Entry method: %d", aEntryMethod);

		RaidcoreAPI = new CApiClient("https://api.raidcore.gg", true, Index(EPath::DIR_APICACHE_RAIDCORE), 30 * 60, 300, 5, 1);//, Certs::Raidcore);
		GitHubAPI = new CApiClient("https://api.github.com", true, Index(EPath::DIR_APICACHE_GITHUB), 30 * 60, 60, 60, 60 * 60);

		std::thread([logger, updater]()
		{
			HANDLE hMutex = CreateMutexA(0, true, "RCGG-Mutex-Patch-Nexus");

			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				logger->Info(CH_CORE, "Cannot patch Nexus, mutex locked.");

				/* sanity check to make the compiler happy */
				if (hMutex)
				{
					CloseHandle(hMutex);
				}

				return;
			}

			/* perform/check for update */
			updater->UpdateNexus();

			/* sanity check to make the compiler happy */
			if (hMutex)
			{
				ReleaseMutex(hMutex);
				CloseHandle(hMutex);
			}
		}).detach();

		/* Only initalise if not vanilla */
		if (!CmdLine::HasArgument("-ggvanilla"))
		{
			Multibox::KillMutex();
			logger->Info(CH_CORE, "Multibox State: %d", Multibox::GetState());

			MH_Initialize();

			ctx->GetMumbleReader();

			s_NexusLink = (NexusLinkData_t*)ctx->GetDataLink()->GetResource(DL_NEXUS_LINK);
			s_RawInputApi = ctx->GetRawInputApi();
			s_InputBindApi = ctx->GetInputBindApi();
			s_UIContext = ctx->GetUIContext();
			s_TextureService = ctx->GetTextureService();
			s_EventApi = ctx->GetEventApi();
			s_SettingsCtx = ctx->GetSettingsCtx();

			State::Nexus = ENexusState::LOADED;
		}
		else
		{
			State::Nexus = ENexusState::SHUTDOWN;
		}
	}

	void Shutdown(unsigned int aReason)
	{
		const char* reasonStr;
		switch (aReason)
		{
			case WM_DESTROY: reasonStr = "WM_DESTROY"; break;
			case WM_CLOSE:   reasonStr = "WM_CLOSE"; break;
			case WM_QUIT:    reasonStr = "WM_QUIT"; break;
			default:         reasonStr = NULLSTR; break;
		}

		CContext* ctx = CContext::GetContext();
		CLogApi* logger = ctx->GetLogger();

		logger->Critical(CH_CORE, String::Format("Main::Shutdown() | Reason: %s", reasonStr).c_str());

		if (State::Nexus < ENexusState::SHUTTING_DOWN)
		{
			State::Nexus = ENexusState::SHUTTING_DOWN;

			// free addons
			Loader::Shutdown();

			ctx->GetUIContext()->Shutdown();

			MH_Uninitialize();

			logger->Info(CH_CORE, "Shutdown performed.");

			State::Nexus = ENexusState::SHUTDOWN;
		}

		/* do not free lib, let the OS handle it. else gw2 crashes on shutdown */
		//if (D3D11Handle) { FreeLibrary(D3D11Handle); }
		//if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
	}

	LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::Nexus != ENexusState::SHUTDOWN)
		{
			assert(s_RawInputApi);
			assert(s_UIContext);
			assert(s_InputBindApi);

			// don't pass to game if loader
			if (Loader::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if wndprocCb or InputBind
			//if (InputCtx->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return; }

			// don't pass to game if custom wndproc
			if (s_RawInputApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if gui
			if (s_UIContext->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if InputBind
			if (s_InputBindApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			if (uMsg == WM_DESTROY || uMsg == WM_QUIT || uMsg == WM_CLOSE)
			{
				Main::Shutdown(uMsg);
			}

			/* offset of 7997, if uMsg in that range it's a nexus game only message */
			if (uMsg >= WM_PASSTHROUGH_FIRST && uMsg <= WM_PASSTHROUGH_LAST)
			{
				/* modify the uMsg code to the original code */
				uMsg -= WM_PASSTHROUGH_FIRST;
			}

			bool lockCursor = s_SettingsCtx ? s_SettingsCtx->Get<bool>(OPT_CAMCTRL_LOCKCURSOR, false) : false;
			bool resetCursor = s_SettingsCtx ? s_SettingsCtx->Get<bool>(OPT_CAMCTRL_RESETCURSOR, false) : false;

			{
				static bool s_IsConfining = false;
				static POINT s_LastPos = {};

				CURSORINFO ci{};
				ci.cbSize = sizeof(CURSORINFO);
				GetCursorInfo(&ci);

				/* Store last visible pos on first click. */
				switch (uMsg)
				{
					case WM_LBUTTONDOWN:
					{
						if (ci.flags != 0 && (wParam & MK_RBUTTON) != MK_RBUTTON)
						{
							s_LastPos = ci.ptScreenPos;
						}
						break;
					}
					case WM_RBUTTONDOWN:
					{
						if (ci.flags != 0 && (wParam & MK_LBUTTON) != MK_LBUTTON)
						{
							s_LastPos = ci.ptScreenPos;
						}
						break;
					}
				}

				switch (uMsg)
				{
					case WM_LBUTTONDOWN:
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_RBUTTONUP:
					case WM_MOUSEMOVE:
					{
						if (s_IsConfining && (ci.flags != 0))
						{
							if (lockCursor)
							{
								ClipCursor(NULL);
							}

							if (resetCursor)
							{
								SetCursorPos(s_LastPos.x, s_LastPos.y);
							}
							s_IsConfining = false;
						}
						else if (!s_IsConfining && (ci.flags == 0) && ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)))
						{
							RECT rect{
								s_LastPos.x,
								s_LastPos.y,
								s_LastPos.x + 1,
								s_LastPos.y + 1
							};
							if (lockCursor)
							{
								ClipCursor(&rect);
							}
							s_IsConfining = true;
						}

						break;
					}
					case WM_ACTIVATEAPP:
					{
						if (s_IsConfining)
						{
							if (lockCursor)
							{
								ClipCursor(NULL);
							}
							if (resetCursor)
							{
								SetCursorPos(s_LastPos.x, s_LastPos.y);
							}
							s_IsConfining = false;
						}

						break;
					}
				}
			}
		}

		return CallWindowProcA(Hooks::WndProc, hWnd, uMsg, wParam, lParam);
	}

	HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
	{
		if (!(State::Nexus == ENexusState::SHUTTING_DOWN || State::Nexus == ENexusState::SHUTDOWN))
		{
			assert(s_UIContext);
			assert(s_TextureService);

			if (s_RendererCtx->SwapChain != pChain)
			{
				s_RendererCtx->SwapChain = pChain;

				if (s_RendererCtx->Device)
				{
					s_RendererCtx->DeviceContext->Release();
					s_RendererCtx->DeviceContext = nullptr;
					s_RendererCtx->Device->Release();
					s_RendererCtx->Device = nullptr;
				}

				s_RendererCtx->SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&s_RendererCtx->Device);
				s_RendererCtx->Device->GetImmediateContext(&s_RendererCtx->DeviceContext);

				DXGI_SWAP_CHAIN_DESC swapChainDesc;
				s_RendererCtx->SwapChain->GetDesc(&swapChainDesc);

				s_RendererCtx->Window.Handle = swapChainDesc.OutputWindow;
				Hooks::WndProc = (WNDPROC)SetWindowLongPtr(s_RendererCtx->Window.Handle, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

				Loader::Initialize();

				State::Nexus = ENexusState::READY;
				State::Directx = EDxState::READY; /* acquired swapchain */
			}

			s_UIContext->Initialize(s_RendererCtx->Window.Handle, s_RendererCtx->Device, s_RendererCtx->DeviceContext, s_RendererCtx->SwapChain);

			Loader::ProcessQueue();

			s_TextureService->Advance();

			s_UIContext->Render();
		}

		s_RendererCtx->Metrics.FrameCount++;

		return Hooks::DXGIPresent(pChain, SyncInterval, Flags);
	}

	HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		assert(s_UIContext);
		assert(s_NexusLink);
		assert(s_EventApi);

		s_UIContext->Shutdown();

		/* Cache window dimensions */
		s_RendererCtx->Window.Width = Width;
		s_RendererCtx->Window.Height = Height;

		if (s_NexusLink)
		{
			/* Already write to nexus link, as addons depend on that and the next frame isn't called yet so no update to values */
			s_NexusLink->Width = Width;
			s_NexusLink->Height = Height;
		}

		s_EventApi->Raise(EV_WINDOW_RESIZED);

		return Hooks::DXGIResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
}
