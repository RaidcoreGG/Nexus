#include <d3d11.h>
#include <cassert>
#include <thread>
#include <algorithm>

#include "minhook/mh_hook.h"

#include "core.h"
#include "Paths.h"
#include "State.h"
#include "Hooks.h"
#include "Renderer.h"
#include "Shared.h"
#include "Consts.h"

#include "Logging/LogHandler.h"

#include "Mumble/Mumble.h"
#include "WndProc/WndProcHandler.h"
#include "Keybinds/KeybindHandler.h"
#include "Events/EventHandler.h"
#include "GUI/GUI.h"
#include "Loader/Loader.h"
#include "DataLink/DataLink.h"
#include "Textures/TextureLoader.h"
#include "Loader/NexusLinkData.h"
#include "Settings/Settings.h"
#include "API/APIController.h"

void UpdateNexusLink()
{
	NexusLink->Width			= Renderer::Width;
	NexusLink->Height			= Renderer::Height;
	NexusLink->Scaling			= Renderer::Scaling;

	NexusLink->IsMoving			= IsMoving;
	NexusLink->IsCameraMoving	= IsCameraMoving;
	NexusLink->IsGameplay		= IsGameplay;

	NexusLink->Font				= Font;
	NexusLink->FontBig			= FontBig;
	NexusLink->FontUI			= FontUI;
}

void Initialize()
{
	Version->Major = 1;
	Version->Minor = 0;
	Version->Build = 0;
	Version->Revision = 1;

	State::AddonHost = ENexusState::LOAD;

	State::Initialize();
	Path::Initialize(NexusHandle);

	/* Don't initialize anything if vanilla*/
	if (!State::IsVanilla)
	{
		LogHandler::Initialize();

		LogInfo(CH_CORE, GetCommandLineA());
		LogInfo(CH_CORE, "Version: %s", Version->ToString().c_str());

		MH_Initialize();

		Keybinds::Load();
		Settings::Load();

		State::IsDeveloperMode = Settings::Settings[OPT_DEVMODE].get<bool>();
		//API::Initialize();

		Mumble::Initialize();
		NexusLink = (NexusLinkData*)DataLink::ShareResource((DL_NEXUS_LINK_ + std::to_string(GetCurrentProcessId())).c_str(), sizeof(NexusLinkData));

		// create imgui context
		if (!Renderer::GuiContext) { Renderer::GuiContext = ImGui::CreateContext(); }

		Loader::Initialize();
	}
	else
	{
		State::AddonHost = ENexusState::SHUTDOWN;
	}
}
void Shutdown()
{
	LogCritical(CH_CORE, "::Shutdown()");

	if (State::AddonHost < ENexusState::SHUTDOWN)
	{
		State::AddonHost = ENexusState::SHUTDOWN;

		GUI::Shutdown();
		Mumble::Shutdown();

		// free addons & shared mem
		Loader::Shutdown();
		DataLink::Free();

		/* Save keybinds, settings, api keys & api cache */
		Keybinds::Save();
		Settings::Save();
		//API::Save();

		MH_Uninitialize();

		LogHandler::Shutdown();
	}

	// free libs
	if (D3D11Handle) { FreeLibrary(D3D11Handle); }
	if (D3D11SystemHandle) { FreeLibrary(D3D11SystemHandle); }
}

/* hk */
LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// don't pass to game if custom wndproc
	if (WndProc::WndProc(hWnd, uMsg, wParam, lParam) == 0) { return 0; }

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

	if (uMsg == WM_DESTROY) { ::Shutdown(); }

	return CallWindowProcA(Hooks::GW2_WndProc, hWnd, uMsg, wParam, lParam);
}
HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
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
		Hooks::GW2_WndProc = (WNDPROC)SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
	}

	while (Loader::QueuedAddons.size() > 0)
	{
		QueuedAddon q = Loader::QueuedAddons.front();

		switch (q.Action)
		{
			case ELoaderAction::Load:
				Loader::LoadAddon(q.Path);
				break;
			case ELoaderAction::Unload:
				Loader::UnloadAddon(q.Path);
				break;
		}

		Loader::QueuedAddons.erase(Loader::QueuedAddons.begin());
	}

	while (TextureLoader::QueuedTextures.size() > 0)
	{
		TextureLoader::CreateTexture(TextureLoader::QueuedTextures.front());
		TextureLoader::QueuedTextures.erase(TextureLoader::QueuedTextures.begin());
	}

	UpdateNexusLink();

	GUI::Render();

	return Hooks::DXGI_Present(pChain, SyncInterval, Flags);
}
HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	GUI::Shutdown();

	/* Cache window dimensions */
	Renderer::Width = Width;
	Renderer::Height = Height;

	Events::Raise(EV_WINDOW_RESIZED, nullptr);
	
	return Hooks::DXGI_ResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

/* dx */
bool DxLoad()
{
	if (State::Directx < EDxState::DIRECTX_READY)
	{
		State::Directx = EDxState::DIRECTX_LOAD;

		/* attempt to chainload */
		/* sanity check that the current dll isn't the chainload */
		if (Path::F_HOST_DLL != Path::F_CHAINLOAD_DLL)
		{
			LogInfo(CH_CORE, "Attempting to chainload.");

			State::IsChainloading = true;

			D3D11Handle = LoadLibraryA(Path::F_CHAINLOAD_DLL);
		}

		if (!D3D11Handle)
		{
			if (State::IsChainloading)
			{
				LogWarning(CH_CORE, "No chainload found or failed to load.");
			}
			State::IsChainloading = false;

			D3D11Handle = LoadLibraryA(Path::F_SYSTEM_DLL);

			LogDebug(CH_CORE, Path::F_SYSTEM_DLL);

			assert(D3D11Handle && "Could not load system d3d11.dll");

			LogInfo(CH_CORE, "Loaded System DLL: %s", Path::F_SYSTEM_DLL);
		}

		State::Directx = EDxState::DIRECTX_READY;

		if (State::Directx < EDxState::DIRECTX_HOOKED && !State::IsVanilla)
		{
			WNDCLASSEXW wc;
			memset(&wc, 0, sizeof(wc));
			wc.cbSize = sizeof(wc);
			wc.lpfnWndProc = DefWindowProcW;
			wc.hInstance = GetModuleHandleW(0);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
			wc.lpszClassName = L"TempDxWndClass";
			RegisterClassExW(&wc);

			HWND wnd = CreateWindowExW(0, wc.lpszClassName, 0, WS_OVERLAPPEDWINDOW, 0, 0, 128, 128, 0, 0, wc.hInstance, 0);
			if (wnd)
			{
				DXGI_SWAP_CHAIN_DESC swap_desc = {};
				swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swap_desc.BufferCount = 1;
				swap_desc.SampleDesc.Count = 1;
				swap_desc.OutputWindow = wnd;
				swap_desc.Windowed = TRUE;

				ID3D11Device* device;
				ID3D11DeviceContext* context;
				IDXGISwapChain* swap;

				if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swap_desc, &swap, &device, 0, &context)))
				{
					LPVOID* vtbl;

					vtbl = *(LPVOID**)swap;
					//dxgi_release = hook_vtbl_fn(vtbl, 2, dxgi_release_hook);
					//dxgi_present = hook_vtbl_fn(vtbl, 8, dxgi_present_hook);
					//dxgi_resize_buffers = hook_vtbl_fn(vtbl, 13, dxgi_resize_buffers_hook);

					MH_CreateHook(vtbl[8], (LPVOID)&hkDXGIPresent, (LPVOID*)&Hooks::DXGI_Present);
					MH_CreateHook(vtbl[13], (LPVOID)&hkDXGIResizeBuffers, (LPVOID*)&Hooks::DXGI_ResizeBuffers);
					MH_EnableHook(MH_ALL_HOOKS);

					context->Release();
					device->Release();
					swap->Release();
				}

				DestroyWindow(wnd);
			}

			UnregisterClassW(wc.lpszClassName, wc.hInstance);

			State::AddonHost = ENexusState::UI_READY;
			State::Directx = EDxState::DIRECTX_HOOKED;
		}
	}

	return (D3D11Handle != NULL);
}

HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CREATEDEVICE; }

	static decltype(&D3D11CreateDevice) func;
	static const char* func_name = "D3D11CreateDevice";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::DIRECTX_READY)
	{
		LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

		D3D11SystemHandle = LoadLibraryA(Path::F_SYSTEM_DLL);

		if (FindFunction(D3D11SystemHandle, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(D3D11Handle, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CREATEDEVICEANDSWAPCHAIN; }

	static decltype(&D3D11CreateDeviceAndSwapChain) func;
	static const char* func_name = "D3D11CreateDeviceAndSwapChain";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::DIRECTX_READY)
	{
		LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

		D3D11SystemHandle = LoadLibraryA(Path::F_SYSTEM_DLL);

		if (FindFunction(D3D11SystemHandle, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(D3D11Handle, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);

	//return DxLoad() ? func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext)
	//				: 0;
}
HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_CREATEDEVICE; }

	static decltype(&D3D11CoreCreateDevice) func;
	static const char* func_name = "D3D11CoreCreateDevice";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::DIRECTX_READY)
	{
		LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

		D3D11SystemHandle = LoadLibraryA(Path::F_SYSTEM_DLL);

		if (FindFunction(D3D11SystemHandle, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(D3D11Handle, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
}
HRESULT __stdcall D3D11CoreCreateLayeredDevice(const void* unknown0, DWORD unknown1, const void* unknown2, REFIID riid, void** ppvObj)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_CREATELAYEREDDEVICE; }

	static decltype(&D3D11CoreCreateLayeredDevice) func;
	static const char* func_name = "D3D11CoreCreateLayeredDevice";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::DIRECTX_READY)
	{
		LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

		D3D11SystemHandle = LoadLibraryA(Path::F_SYSTEM_DLL);

		if (FindFunction(D3D11SystemHandle, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(D3D11Handle, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(unknown0, unknown1, unknown2, riid, ppvObj);
}
SIZE_T	__stdcall D3D11CoreGetLayeredDeviceSize(const void* unknown0, DWORD unknown1)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_GETLAYEREDDEVICESIZE; }

	static decltype(&D3D11CoreGetLayeredDeviceSize) func;
	static const char* func_name = "D3D11CoreGetLayeredDeviceSize";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::DIRECTX_READY)
	{
		LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

		D3D11SystemHandle = LoadLibraryA(Path::F_SYSTEM_DLL);

		if (FindFunction(D3D11SystemHandle, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(D3D11Handle, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(unknown0, unknown1);
}
HRESULT __stdcall D3D11CoreRegisterLayers(const void* unknown0, DWORD unknown1)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_REGISTERLAYERS; }

	static decltype(&D3D11CoreRegisterLayers) func;
	static const char* func_name = "D3D11CoreRegisterLayers";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::DIRECTX_READY)
	{
		LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

		D3D11SystemHandle = LoadLibraryA(Path::F_SYSTEM_DLL);

		if (FindFunction(D3D11SystemHandle, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(D3D11Handle, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(unknown0, unknown1);
}

/* entry */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		NexusHandle = hModule;
		GameHandle = GetModuleHandle(NULL);

		::Initialize();
		break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return true;
}