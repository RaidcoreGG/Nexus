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

#include "Host/GW2/Context.h"
#include "Host/GW2/Main.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Core/Logging/LogApi.h"
#include "Host/GW2/Inputs/GameBinds/GbApi.h"
#include "Host/GW2/Inputs/MouseResetFix.h"
#include "HkFuncDefs.h"
#include "HkGameWindow.h"
#include "HkNotify.h"
#include "UI/Textures/TxLoader.h"
#include "UI/UiContext.h"

namespace Hooks
{
	///----------------------------------------------------------------------------------------------------
	/// CDxgiNotifier Class
	/// 	Source of the swap chain notifications raised by the detours.
	///----------------------------------------------------------------------------------------------------
	class CDxgiNotifier : public IDxgiPresentNotifier, public IDxgiResizeNotifier
	{
		public:
		void Present(IDXGISwapChain* aSwapChain)
		{
			this->NotifyDxgiPresent(aSwapChain);
		}

		void Resize(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight)
		{
			this->NotifyDxgiResize(aSwapChain, aWidth, aHeight);
		}
	};

	static CDxgiNotifier s_DxgiNotifier;

	///----------------------------------------------------------------------------------------------------
	/// CWndProcNotifier Class
	/// 	Source of the window message notification raised by the window procedure detour.
	///----------------------------------------------------------------------------------------------------
	class CWndProcNotifier : public IWndProcNotifier
	{
		public:
		UINT WndProc(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam)
		{
			return this->NotifyWndProc(hWnd, uMsg, wParam, lParam);
		}
	};

	static CWndProcNotifier s_WndProcNotifier;

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

			LogCritical(CH_CORE, "Failed creating temporary window.");
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

			LogDebug(CH_CORE, "HOOK BEGIN");

			/* Create and enable VT hooks. */
			/* Follow the jump chain to work nicely with various other hooks. */
			
			if (vtbl)
			{
				MH_CreateHook(FollowJmpChain((PBYTE)vtbl[8]), (LPVOID)&Detour::DXGIPresent, (LPVOID*)&Target::DXGIPresent);
				MH_CreateHook(FollowJmpChain((PBYTE)vtbl[13]), (LPVOID)&Detour::DXGIResizeBuffers, (LPVOID*)&Target::DXGIResizeBuffers);
			}

			/*if (vtbl1)
			{
				MH_CreateHook(FollowJmpChain((PBYTE)vtbl1[22]), (LPVOID)&Detour::DXGIPresent1, (LPVOID*)&Target::DXGIPresent1);
				swap1->Release();
			}*/

			/*if (vtbl3)
			{
				MH_CreateHook(FollowJmpChain((PBYTE)vtbl3[26]), (LPVOID)&Detour::DXGIResizeBuffers1, (LPVOID*)&Target::DXGIResizeBuffers1);
				swap3->Release();
			}*/

			MH_EnableHook(MH_ALL_HOOKS);

			LogDebug(CH_CORE, "HOOK END");

			/* Release the temporary interfaces. */
			context->Release();
			device->Release();
			swap->Release();
		}
		else
		{
			LogCritical(CH_CORE, "Failed to create D3D11 device and swapchain.");
		}

		/* Destroy the temporary window. */
		DestroyWindow(wnd);
	}

	namespace Target
	{
		WNDPROC          WndProc            = nullptr;
		DXPRESENT        DXGIPresent        = nullptr;
		DXRESIZEBUFFERS  DXGIResizeBuffers  = nullptr;
	}

	namespace Detour
	{
		LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			static CContext* s_Context = CContext::GetContext();

			/* Created here, so they are subscribed before the first message is notified. */
			static CRawInputApi*   s_RawInputApi   = s_Context->GetRawInputApi();
			static CGameBindsApi*  s_GameBindsApi  = s_Context->GetGameBindsApi();
			static CMouseResetFix* s_MouseResetFix = s_Context->GetMouseResetFix();

			if (s_WndProcNotifier.WndProc(hWnd, uMsg, wParam, lParam) == NOTIFY_CONSUMED) { return 0; }

			return CallWindowProcA(Target::WndProc, hWnd, uMsg, wParam, lParam);
		}

		HRESULT __stdcall DXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
		{
			static CContext* s_Context = CContext::GetContext();

			/* Created here, so they are subscribed before the first present is notified. */
			static CGameWindow* s_GameWindow = s_Context->GetGameWindow();
			static CTextureLoader* s_TextureLoader = s_Context->GetTextureService();
			static CUiContext* s_UIContext = s_Context->GetUIContext();

			s_DxgiNotifier.Present(pChain);

			return Target::DXGIPresent(pChain, SyncInterval, Flags);
		}

		HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
		{
			/* The game may resize before its first present. */
			static CGameWindow* s_GameWindow = CContext::GetContext()->GetGameWindow();

			s_DxgiNotifier.Resize(pChain, Width, Height);

			return Target::DXGIResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		}
	}
}
