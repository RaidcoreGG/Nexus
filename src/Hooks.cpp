#include "Hooks.h"

#include "Main.h"

#include "Consts.h"
#include "Renderer.h"
#include "Context.h"
#include "State.h"

namespace Hooks
{
	NexusLinkData*  NexusLink      = nullptr;
	CRawInputApi*   RawInputApi    = nullptr;
	CInputBindApi*  InputBindApi   = nullptr;
	CUiContext*     UIContext      = nullptr;
	CTextureLoader* TextureService = nullptr;
	CEventApi*      EventApi       = nullptr;

	namespace DXGI
	{
		DXPRESENT       Present       = nullptr;
		DXRESIZEBUFFERS ResizeBuffers = nullptr;
	}

	namespace GW2
	{
		WNDPROC         WndProc = nullptr;
	}

	LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::Nexus != ENexusState::SHUTDOWN)
		{
			// don't pass to game if loader
			if (Loader::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if wndprocCb or InputBind
			//if (InputCtx->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return; }

			// don't pass to game if custom wndproc
			if (RawInputApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if gui
			if (UIContext->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if InputBind
			if (InputBindApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

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
		
		return CallWindowProcA(Hooks::GW2::WndProc, hWnd, uMsg, wParam, lParam);
	}
	HRESULT __stdcall DXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
	{
		if (!(State::Nexus == ENexusState::SHUTTING_DOWN || State::Nexus == ENexusState::SHUTDOWN))
		{
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
				Hooks::GW2::WndProc = (WNDPROC)SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)WndProc);

				Loader::Initialize();

				State::Nexus = ENexusState::READY;
				State::Directx = EDxState::READY; /* acquired swapchain */
			}

			UIContext->Initialize(Renderer::WindowHandle, Renderer::Device, Renderer::DeviceContext, Renderer::SwapChain);

			Loader::ProcessQueue();

			TextureService->Advance();

			UIContext->Render();
		}
		
		Renderer::FrameCounter++;

		return Hooks::DXGI::Present(pChain, SyncInterval, Flags);
	}
	HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		UIContext->Shutdown();

		/* Cache window dimensions */
		Renderer::Width = Width;
		Renderer::Height = Height;

		if (NexusLink)
		{
			/* Already write to nexus link, as addons depend on that and the next frame isn't called yet so no update to values */
			NexusLink->Width = Width;
			NexusLink->Height = Height;
		}

		EventApi->Raise(EV_WINDOW_RESIZED);

		return Hooks::DXGI::ResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
}
