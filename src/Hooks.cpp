#include "Hooks.h"

#include "Main.h"

#include "Consts.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"

#include "Events/EventHandler.h"
#include "GUI/GUI.h"
#include "GUI/Widgets/QuickAccess/QuickAccess.h"
#include "Inputs/Keybinds/KeybindHandler.h"
#include "Loader/Loader.h"
#include "Loader/NexusLinkData.h"
#include "Services/Textures/TextureLoader.h"
#include "Inputs/WndProc/WndProcHandler.h"

namespace Hooks
{
	NexusLinkData* NexusLink = nullptr;

	namespace DXGI
	{
		DXPRESENT		Present = nullptr;
		DXRESIZEBUFFERS	ResizeBuffers = nullptr;
	}

	namespace GW2
	{
		WNDPROC			WndProc = nullptr;
	}

	LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::Nexus != ENexusState::SHUTDOWN)
		{
			// don't pass to game if loader
			if (Loader::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if custom wndproc
			if (RawInputApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if keybind
			if (Keybinds::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if gui
			if (GUI::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass keys to game if currently editing keybinds
			if (Keybinds::IsSettingKeybind)
			{
				if (uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYUP || uMsg == WM_KEYUP)
				{
					return 0;
				}
			}

			if (uMsg == WM_DESTROY || uMsg == WM_QUIT || uMsg == WM_CLOSE)
			{
				Main::Shutdown(uMsg);
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

			if (State::Nexus == ENexusState::READY && !State::IsImGuiInitialized)
			{
				GUI::Initialize();
			}

			Loader::ProcessQueue();

			TextureService->Advance();

			if (NexusLink)
			{
				NexusLink->Scaling = Renderer::Scaling;

				NexusLink->Font = Font;
				NexusLink->FontBig = FontBig;
				NexusLink->FontUI = FontUI;

				NexusLink->QuickAccessIconsCount = static_cast<int>(GUI::QuickAccess::Registry.size()); // write this only when adding/removing icons
				NexusLink->QuickAccessMode = (int)GUI::QuickAccess::Location;
				NexusLink->QuickAccessIsVertical = GUI::QuickAccess::VerticalLayout;
			}

			GUI::Render();
		}
		
		Renderer::FrameCounter++;

		return Hooks::DXGI::Present(pChain, SyncInterval, Flags);
	}
	HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		GUI::Shutdown();

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
