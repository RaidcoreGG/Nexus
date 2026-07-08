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
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Logging/LogApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "GW2/Inputs/MouseResetFix.h"
#include "HkConst.h"
#include "HkFuncDefs.h"
#include "UI/Renderer/RdrContext.h"
#include "UI/Textures/TxLoader.h"
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
			CLogApi* logger = ctx.GetLogger();

			logger->Critical(CH_CORE, "Failed creating temporary window.");
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
			CLogApi* logger = ctx.GetLogger();

			logger->Debug(CH_CORE, "HOOK BEGIN");

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

			logger->Debug(CH_CORE, "HOOK END");

			/* Release the temporary interfaces. */
			context->Release();
			device->Release();
			swap->Release();
		}
		else
		{
			Runtime& ctx = Runtime::Get();
			CLogApi* logger = ctx.GetLogger();

			logger->Critical(CH_CORE, "Failed to create D3D11 device and swapchain.");
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
			static Runtime&       s_Context      = Runtime::Get();
			static CInputBindApi* s_InputBindApi = s_Context.GetInputBindApi();
			static CRawInputApi*  s_RawInputApi  = s_Context.GetRawInputApi();
			static CUiContext*    s_UIContext    = s_Context.GetUIContext();
			static CLoader*       s_Loader       = s_Context.GetLoader();
			static CGameBindsApi* s_GameBindsApi = &s_Context.Game().GameBinds();

			// don't pass to game if loader
			if (s_Loader->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if custom wndproc
			if (s_RawInputApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if gui
			if (s_UIContext->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			// don't pass to game if InputBind
			if (s_InputBindApi->WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

			if (uMsg == WM_DESTROY)
			{
				Runtime::Get().Shutdown(uMsg);
			}

			// shift game only messages back to normal messages.
			s_GameBindsApi->RedirectGameOnly(hWnd, uMsg, wParam, lParam);

			MouseResetFix(hWnd, uMsg, wParam, lParam);

			return CallWindowProcA(Target::WndProc, hWnd, uMsg, wParam, lParam);
		}

		void Present_Internal(IDXGISwapChain* aSwapChain)
		{
			static Runtime& s_Context = Runtime::Get();
			static RenderContext_t* s_RenderCtx = s_Context.GetRendererCtx();
			static CTextureLoader* s_TextureLoader = s_Context.GetTextureService();
			static CUiContext* s_UIContext = s_Context.GetUIContext();
			static CLoader* s_Loader = s_Context.GetLoader();

			/* Increment count at the beginning of the frame. */
			s_RenderCtx->Metrics.FrameCount++;

			/* The swap chain we used to hook is different than the one the game created.
			 * To be precise, we should have no swapchain at all right now. */
			if (s_RenderCtx->SwapChain != aSwapChain)
			{
				s_RenderCtx->SwapChain = aSwapChain;

				DXGI_SWAP_CHAIN_DESC swapChainDesc{};
				s_RenderCtx->SwapChain->GetDesc(&swapChainDesc);

				s_RenderCtx->Window.Handle = swapChainDesc.OutputWindow;
				Target::WndProc = (WNDPROC)SetWindowLongPtr(s_RenderCtx->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Detour::WndProc);

				s_Loader->InitDirectoryUpdates(s_RenderCtx->Window.Handle);
			}

			s_TextureLoader->Advance();

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
			static CDataLinkApi* s_DataLink = s_Context.GetDataLink();
			static CEventApi* s_EventApi = s_Context.GetEventApi();
			static RenderContext_t* s_RenderCtx = s_Context.GetRendererCtx();
			static CUiContext* s_UIContext = s_Context.GetUIContext();

			s_UIContext->Shutdown();

			/* Cache window dimensions */
			s_RenderCtx->Window.Width = aWidth;
			s_RenderCtx->Window.Height = aHeight;
			
			NexusLinkData_t* nexuslink = (NexusLinkData_t*)s_DataLink->GetResource(DL_NEXUS_LINK);

			if (nexuslink)
			{
				/* Already write to nexus link, as addons depend on that and the next frame isn't called yet so no update to values */
				nexuslink->Width = aWidth;
				nexuslink->Height = aHeight;
			}

			s_EventApi->Raise(EV_WINDOW_RESIZED);
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
