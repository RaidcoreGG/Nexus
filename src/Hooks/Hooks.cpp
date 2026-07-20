///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Hooks.cpp
/// Description  :  Implementation for hooked/detoured functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Hooks.h"

#include <cstdint>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <windows.h>

#include "minhook/mh_hook.h"

#include "Runtime/Runtime.h"
using namespace Raidcore::Nexus;

#include "Core/NexusLink.h"
#include "Core/DataLink/DlApi.h"
#include "Host/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Host/Loader/Loader.h"
#include "Core/Logging/LogApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Inputs/MouseResetFix.h"
#include "HkConst.h"
#include "HkFuncDefs.h"
#include "Graphics/GrContext.h"
#include "Graphics/Textures/TxLoader.h"
#include "UI/UiContext.h"
#include "Util/CmdLine.h"

namespace Hooks
{
	inline PBYTE FollowJmpChain(PBYTE aPointer)
	{
		while (true)
		{
			if (aPointer[0] == 0xE9)
			{
				/* near jmp */
				/* address is relative to after jmp */
				aPointer += 5 + *(__unaligned signed int*) & aPointer[1]; // jmp +imm32
			}
			else if (aPointer[0] == 0xFF && aPointer[1] == 0x25)
			{
				/* far jmp */
				/* x64: address is relative to after jmp */
				/* x86: absolute address can be read directly */
#ifdef _WIN64
				aPointer += 6 + *(__unaligned signed int*) & aPointer[2]; // jmp [+imm32]
#else
				aPointer = *(__unaligned signed int*) & aPointer[2]; // jmp [imm32]
#endif
				/* dereference to get the actual target address */
				aPointer = *(__unaligned PBYTE*)aPointer;
			}
			else
			{
				break;
			}
		}

		return aPointer;
	}

	void HookIDXGISwapChain()
	{
		if (CmdLine::HasArgument("-ggvanilla"))                             { return; }
		if (Hooks::Target::DXGIPresent && Hooks::Target::DXGIResizeBuffers) { return; }

		/* Create and register window class for temp window. */
		WNDCLASSEXA wc{};
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = DefWindowProcA;
		wc.hInstance = GetModuleHandleA(0);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wc.lpszClassName = "Raidcore_Dx_Window_Class";
		RegisterClassExA(&wc);

		/* Create the temp window. */
		HWND wnd = CreateWindowExA(0, wc.lpszClassName, 0, WS_OVERLAPPED, 0, 0, 1280, 720, 0, 0, wc.hInstance, 0);

		/* If window creation failed, deregister the class and log an error. */
		if (!wnd)
		{
			UnregisterClassA(wc.lpszClassName, wc.hInstance);

			Runtime& ctx    = Runtime::Get();
			Core::LogApi& logger = ctx.Core().Logger();

			logger.Critical(CH_CORE, "Failed creating temporary window.");
			return;
		}

		/* Create description for our temp swapchain. */
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.OutputWindow = wnd;
		swapChainDesc.Windowed = TRUE;

		/* Temporary interface pointers used to hook. */
		ID3D11Device*        device  = nullptr;
		ID3D11DeviceContext* context = nullptr;
		IDXGISwapChain*      swap    = nullptr;

		/* Create the swapchain. */
		if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swapChainDesc, &swap, &device, 0, &context)))
		{
			IDXGISwapChain1* swap1 = nullptr;
			IDXGISwapChain3* swap3 = nullptr;

			swap->QueryInterface(IID_PPV_ARGS(&swap1));
			swap->QueryInterface(IID_PPV_ARGS(&swap3));

			LPVOID* vtbl = *(LPVOID**)swap;
			LPVOID* vtbl1 = swap1 ? *reinterpret_cast<LPVOID**>(swap1) : nullptr;
			LPVOID* vtbl3 = swap3 ? *reinterpret_cast<LPVOID**>(swap3) : nullptr;

			Runtime& ctx = Runtime::Get();
			Core::LogApi& logger = ctx.Core().Logger();

			logger.Debug(CH_CORE, "HOOK BEGIN");

			/* Create and enable VT hooks. */
			/* Follow the jump chain to work nicely with various other hooks. */
			MH_CreateHook(FollowJmpChain((PBYTE)vtbl[8]),  (LPVOID)&Detour::DXGIPresent,       (LPVOID*)&Target::DXGIPresent      );
			MH_CreateHook(FollowJmpChain((PBYTE)vtbl[13]), (LPVOID)&Detour::DXGIResizeBuffers, (LPVOID*)&Target::DXGIResizeBuffers);

			if (vtbl1)
			{
				MH_CreateHook(FollowJmpChain((PBYTE)vtbl1[22]), (LPVOID)&Detour::DXGIPresent1, (LPVOID*)&Target::DXGIPresent1);
				swap1->Release();
			}

			if (vtbl3)
			{
				MH_CreateHook(FollowJmpChain((PBYTE)vtbl3[26]), (LPVOID)&Detour::DXGIResizeBuffers1, (LPVOID*)&Target::DXGIResizeBuffers1);
				swap3->Release();
			}

			MH_EnableHook(MH_ALL_HOOKS);

			logger.Debug(CH_CORE, "HOOK END");

			/* Release the temporary interfaces. */
			context->Release();
			device->Release();
			swap->Release();
		}
		else
		{
			Runtime& ctx = Runtime::Get();
			Core::LogApi& logger = ctx.Core().Logger();

			logger.Critical(CH_CORE, "Failed to create D3D11 device and swapchain.");
		}

		/* Destroy the temporary window. */
		DestroyWindow(wnd);
	}

	namespace Target
	{
		WNDPROC          WndProc            = nullptr;
		DXPRESENT        DXGIPresent        = nullptr;
		DXPRESENT1       DXGIPresent1       = nullptr;
		DXRESIZEBUFFERS  DXGIResizeBuffers  = nullptr;
		DXRESIZEBUFFERS1 DXGIResizeBuffers1 = nullptr;
	}

	namespace Detour
	{
		LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			static Runtime&               s_Context      = Runtime::Get();
			static CInputBindApi*         s_InputBindApi = s_Context.GetInputBindApi();
			static Platform::RawInputApi& s_RawInputApi  = s_Context.Platform().RawInput();
			static GUI::CUiContext*            s_UIContext    = s_Context.GetUIContext();
			static Host::Loader&         s_Loader       = s_Context.Host().Loader();
			static GW2::GameBindsApi&     s_GameBindsApi = s_Context.Game().GameBinds();

			// don't pass to game if loader
			if (s_Loader.WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if custom wndproc
			if (s_RawInputApi.WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if gui
			if (s_UIContext->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if InputBind
			if (s_InputBindApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			if (uMsg == WM_DESTROY)
			{
				Runtime::Get().Shutdown(uMsg);
			}

			// shift game only messages back to normal messages.
			s_GameBindsApi.RedirectGameOnly(hWnd, uMsg, wParam, lParam);

			GW2::MouseResetFix(hWnd, uMsg, wParam, lParam);

			return CallWindowProcA(Target::WndProc, hWnd, uMsg, wParam, lParam);
		}

		void Present_Internal(IDXGISwapChain* aSwapChain)
		{
			static Runtime& s_Context = Runtime::Get();
			static Graphics::Metrics_t s_GrMetrics = s_Context.Graphics().Metrics();
			static Graphics::Window_t& s_GrWindow = s_Context.Graphics().Window();
			static Graphics::TextureLoader& s_TextureLoader = s_Context.Graphics().Textures();
			static GUI::CUiContext* s_UIContext = s_Context.GetUIContext();
			static Host::Loader& s_Loader = s_Context.Host().Loader();

			/* Increment count at the beginning of the frame. */
			s_GrMetrics.FrameCount++;

			/* The swap chain we used to hook is different than the one the game created.
			 * To be precise, we should have no swapchain at all right now. */
			if (s_GrWindow.SwapChain != aSwapChain)
			{
				s_GrWindow.SwapChain = aSwapChain;

				DXGI_SWAP_CHAIN_DESC swapChainDesc{};
				s_GrWindow.SwapChain->GetDesc(&swapChainDesc);

				Target::WndProc = (WNDPROC)SetWindowLongPtr(swapChainDesc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)Detour::WndProc);

				s_Loader.InitDirectoryUpdates(swapChainDesc.OutputWindow);
			}

			s_TextureLoader.Advance();

			s_UIContext->Render();
		}

		HRESULT __stdcall DXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
		{
			Present_Internal(pChain);
			return Target::DXGIPresent(pChain, SyncInterval, Flags);
		}

		HRESULT __stdcall DXGIPresent1(IDXGISwapChain1* pChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
		{
			Present_Internal(pChain);
			return Target::DXGIPresent1(pChain, SyncInterval, PresentFlags, pPresentParameters);
		}

		void Resize_Internal(uint32_t aWidth, uint32_t aHeight)
		{
			static Runtime& s_Context = Runtime::Get();
			static Core::DataLinkApi& s_DataLink = s_Context.Core().DataLink();
			static Host::EventApi& s_EventApi = s_Context.Host().Events();
			static Graphics::Window_t& s_GrWindow = s_Context.Graphics().Window();
			static GUI::CUiContext* s_UIContext = s_Context.GetUIContext();

			s_UIContext->Shutdown();

			/* Cache window dimensions */
			s_GrWindow.Width = aWidth;
			s_GrWindow.Height = aHeight;
			
			NexusLinkData_t* nexuslink = (NexusLinkData_t*)s_DataLink.GetResource(DL_NEXUS_LINK);

			if (nexuslink)
			{
				/* Already write to nexus link, as addons depend on that and the next frame isn't called yet so no update to values */
				nexuslink->Width = aWidth;
				nexuslink->Height = aHeight;
			}

			s_EventApi.Raise(EV_WINDOW_RESIZED);
		}

		HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
		{
			Resize_Internal(Width, Height);
			return Target::DXGIResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		}

		HRESULT __stdcall DXGIResizeBuffers1(IDXGISwapChain3* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
		{
			Resize_Internal(Width, Height);
			return Target::DXGIResizeBuffers1(pChain, BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
		}
	}
}
