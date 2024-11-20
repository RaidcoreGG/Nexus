#include "Main.h"

#include <filesystem>
#include <Psapi.h>
#include <string>

#include "Consts.h"
#include "Index.h"
#include "Renderer.h"
#include "resource.h"
#include "Shared.h"
#include "State.h"

#include "Context.h"
#include "Inputs/GameBinds/GameBindsHandler.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"
#include "Services/API/ApiClient.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Logging/CConsoleLogger.h"
#include "Services/Logging/CFileLogger.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Multibox/Multibox.h"
#include "Services/Mumble/Reader.h"
#include "Services/Settings/Settings.h"
#include "Services/Updater/Updater.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"

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
			/* cleanly remove wndproc hook */
			if (Renderer::WindowHandle && Hooks::WndProc)
			{
				SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)Hooks::WndProc);
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
	static NexusLinkData*  s_NexusLink      = nullptr;
	static CRawInputApi*   s_RawInputApi    = nullptr;
	static CInputBindApi*  s_InputBindApi   = nullptr;
	static CUiContext*     s_UIContext      = nullptr;
	static CTextureLoader* s_TextureService = nullptr;
	static CEventApi*      s_EventApi       = nullptr;

	void Initialize(EEntryMethod aEntryMethod)
	{
		if (State::Nexus >= ENexusState::LOAD) { return; }

		State::Nexus = ENexusState::LOAD;

		CContext* ctx = CContext::GetContext();

		//SetUnhandledExceptionFilter(UnhandledExcHandler);
		
		Index::BuildIndex(ctx->GetModule());
		
		CLogHandler* logger = ctx->GetLogger();
		CUpdater* updater = ctx->GetUpdater();

		/* setup default loggers */
		if (CmdLine::HasArgument("-ggconsole"))
		{
			logger->RegisterLogger(new CConsoleLogger(ELogLevel::ALL));
		}

		std::filesystem::path logPathOverride;

		if (CmdLine::HasArgument("-mumble"))
		{
			logPathOverride = (Index::D_GW2_ADDONS_NEXUS / "Nexus_").string() + CmdLine::GetArgumentValue("-mumble") + ".log";
		}

		logger->RegisterLogger(new CFileLogger(ELogLevel::ALL, logPathOverride.empty() ? Index::F_LOG : logPathOverride));

		logger->Info(CH_CORE, GetCommandLineA());
		logger->Info(CH_CORE, "%s: %s", Index::F_HOST_DLL != Index::F_CHAINLOAD_DLL ? "Proxy" : "Chainload", Index::F_HOST_DLL.string().c_str());
		logger->Info(CH_CORE, "Nexus: %s %s", ctx->GetVersion().string().c_str(), ctx->GetBuild());
		logger->Info(CH_CORE, "Entry method: %d", aEntryMethod);

		RaidcoreAPI = new CApiClient("https://api.raidcore.gg", true, Index::D_GW2_ADDONS_COMMON_API_RAIDCORE, 30 * 60, 300, 5, 1);//, Certs::Raidcore);
		GitHubAPI = new CApiClient("https://api.github.com", true, Index::D_GW2_ADDONS_COMMON_API_GITHUB, 30 * 60, 60, 60, 60 * 60);

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
			Multibox::ShareArchive();
			Multibox::ShareLocal();
			Multibox::KillMutex();
			logger->Info(CH_CORE, "Multibox State: %d", Multibox::GetState());

			MH_Initialize();

			ctx->GetMumbleReader();

			s_NexusLink = (NexusLinkData*)ctx->GetDataLink()->GetResource(DL_NEXUS_LINK);
			s_RawInputApi = ctx->GetRawInputApi();
			s_InputBindApi = ctx->GetInputBindApi();
			s_UIContext = ctx->GetUIContext();
			s_TextureService = ctx->GetTextureService();
			s_EventApi = ctx->GetEventApi();

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
		CLogHandler* logger = ctx->GetLogger();

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
		}

		return CallWindowProcA(Hooks::WndProc, hWnd, uMsg, wParam, lParam);
	}

	HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
	{
		if (!(State::Nexus == ENexusState::SHUTTING_DOWN || State::Nexus == ENexusState::SHUTDOWN))
		{
			assert(s_UIContext);
			assert(s_TextureService);

			if (Renderer::SwapChain != pChain)
			{
				Renderer::SwapChain = pChain;

				if (Renderer::Device)
				{
					Renderer::DeviceContext->Release();
					Renderer::DeviceContext = 0;
					Renderer::Device->Release();
					Renderer::Device = 0;
				}

				Renderer::SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Renderer::Device);
				Renderer::Device->GetImmediateContext(&Renderer::DeviceContext);

				DXGI_SWAP_CHAIN_DESC swapChainDesc;
				Renderer::SwapChain->GetDesc(&swapChainDesc);

				Renderer::WindowHandle = swapChainDesc.OutputWindow;
				Hooks::WndProc = (WNDPROC)SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

				Loader::Initialize();

				State::Nexus = ENexusState::READY;
				State::Directx = EDxState::READY; /* acquired swapchain */
			}

			s_UIContext->Initialize(Renderer::WindowHandle, Renderer::Device, Renderer::DeviceContext, Renderer::SwapChain);

			Loader::ProcessQueue();

			s_TextureService->Advance();

			s_UIContext->Render();
		}

		Renderer::FrameCounter++;

		return Hooks::DXGIPresent(pChain, SyncInterval, Flags);
	}

	HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		assert(s_UIContext);
		assert(s_NexusLink);
		assert(s_EventApi);

		s_UIContext->Shutdown();

		/* Cache window dimensions */
		Renderer::Width = Width;
		Renderer::Height = Height;

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
