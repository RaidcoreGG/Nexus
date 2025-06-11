///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Hooks.cpp
/// Description  :  Implementation for hooked/detoured functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Hooks.h"

/* FIXME: Legacy garbage. Need to refactor. */
#include "State.h"

#include "Core/Context.h"
#include "Core/Main.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Loader/NexusLinkData.h"
#include "Engine/Renderer/RdrContext.h"
#include "Engine/Textures/TxLoader.h"
#include "GW2/Inputs/MouseResetFix.h"
#include "HkConst.h"
#include "UI/UiContext.h"

namespace Hooks
{
	namespace Target
	{
		DXPRESENT       DXGIPresent       = nullptr;
		DXRESIZEBUFFERS DXGIResizeBuffers = nullptr;
		WNDPROC         WndProc           = nullptr;
	}

	namespace Detour
	{
		LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (State::Nexus != ENexusState::SHUTDOWN)
			{
				static CContext*        s_Context      = CContext::GetContext();
				static CInputBindApi*   s_InputBindApi = s_Context->GetInputBindApi();
				static CRawInputApi*    s_RawInputApi  = s_Context->GetRawInputApi();
				static CUiContext*      s_UIContext    = s_Context->GetUIContext();

				// don't pass to game if loader
				if (Loader::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

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

				MouseResetFix(hWnd, uMsg, wParam, lParam);
			}

			return CallWindowProcA(Target::WndProc, hWnd, uMsg, wParam, lParam);
		}

		HRESULT __stdcall DXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
		{
			if (!(State::Nexus == ENexusState::SHUTTING_DOWN || State::Nexus == ENexusState::SHUTDOWN))
			{
				static CContext*        s_Context       = CContext::GetContext();
				static RenderContext_t* s_RenderCtx     = s_Context->GetRendererCtx();
				static CTextureLoader*  s_TextureLoader = s_Context->GetTextureService();
				static CUiContext*      s_UIContext     = s_Context->GetUIContext();

				/* The swap chain we used to hook is different than the one the game created.
				 * To be precise, we should have no swapchain at all right now. */
				if (s_RenderCtx->SwapChain != pChain)
				{
					s_RenderCtx->SwapChain = pChain;

					if (s_RenderCtx->Device)
					{
						/* Sanity check. If we have a device, we should also have a context. */
						if (s_RenderCtx->DeviceContext)
						{
							s_RenderCtx->DeviceContext->Release();
							s_RenderCtx->DeviceContext = nullptr;
						}

						s_RenderCtx->Device->Release();
						s_RenderCtx->Device = nullptr;
					}

					s_RenderCtx->SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&s_RenderCtx->Device);
					s_RenderCtx->Device->GetImmediateContext(&s_RenderCtx->DeviceContext);

					DXGI_SWAP_CHAIN_DESC swapChainDesc{};
					s_RenderCtx->SwapChain->GetDesc(&swapChainDesc);

					s_RenderCtx->Window.Handle = swapChainDesc.OutputWindow;
					Target::WndProc = (WNDPROC)SetWindowLongPtr(s_RenderCtx->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Detour::WndProc);

					Loader::Initialize();

					State::Nexus = ENexusState::READY;
					State::Directx = EDxState::READY;
				}

				Loader::ProcessQueue();

				s_TextureLoader->Advance();

				s_UIContext->Render();

				s_RenderCtx->Metrics.FrameCount++;
			}

			return Target::DXGIPresent(pChain, SyncInterval, Flags);
		}

		HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
		{
			if (!(State::Nexus == ENexusState::SHUTTING_DOWN || State::Nexus == ENexusState::SHUTDOWN))
			{
				static CContext*        s_Context   = CContext::GetContext();
				static CDataLinkApi*    s_DataLink  = s_Context->GetDataLink();
				static CEventApi*       s_EventApi  = s_Context->GetEventApi();
				static RenderContext_t* s_RenderCtx = s_Context->GetRendererCtx();
				static CUiContext*      s_UIContext = s_Context->GetUIContext();

				s_UIContext->Shutdown();

				/* Cache window dimensions */
				s_RenderCtx->Window.Width = Width;
				s_RenderCtx->Window.Height = Height;

				NexusLinkData_t* nexuslink = (NexusLinkData_t*)s_DataLink->GetResource(DL_NEXUS_LINK);

				if (nexuslink)
				{
					/* Already write to nexus link, as addons depend on that and the next frame isn't called yet so no update to values */
					nexuslink->Width = Width;
					nexuslink->Height = Height;
				}

				s_EventApi->Raise(EV_WINDOW_RESIZED);
			}

			return Target::DXGIResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		}
	}
}
