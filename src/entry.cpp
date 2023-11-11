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
	State::Nexus = ENexusState::LOAD;

	Version->Major = __TIME_YEARS__;
	Version->Minor = __TIME_MONTH__;
	Version->Build = __TIME_DAYS__;
	Version->Revision = (__TIME_HOURS__ * 60) + (__TIME_MINUTES__);

	LogInfo(CH_CORE, GetCommandLineA());
	LogInfo(CH_CORE, "Version: %s", Version->ToString().c_str());

	State::Initialize();
	Path::Initialize(NexusHandle);

	/* Don't initialize anything if vanilla*/
	if (!State::IsVanilla)
	{
		LogHandler::Initialize();

		MH_Initialize();

		Keybinds::Initialize();
		Keybinds::Load();
		Settings::Load();

		if (!Settings::Settings[OPT_DEVMODE].is_null()) { State::IsDeveloperMode = Settings::Settings[OPT_DEVMODE].get<bool>(); }
		//API::Initialize();

		Mumble::Initialize();
		NexusLink = (NexusLinkData*)DataLink::ShareResource((DL_NEXUS_LINK_ + std::to_string(GetCurrentProcessId())).c_str(), sizeof(NexusLinkData));

		// create imgui context
		if (!Renderer::GuiContext) { Renderer::GuiContext = ImGui::CreateContext(); }

		Loader::Initialize();

		State::Nexus = ENexusState::LOADED;
	}
	else
	{
		State::Nexus = ENexusState::SHUTDOWN;
	}
}
void Shutdown()
{
	LogCritical(CH_CORE, "::Shutdown()");

	if (State::Nexus < ENexusState::SHUTDOWN)
	{
		State::Nexus = ENexusState::SHUTDOWN;

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

bool hook = false;

/* hk */
LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == 77)
	{
		hook = true;
	}

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

		State::Nexus = ENexusState::READY;
		State::Directx = EDxState::READY; /* acquired swapchain */
	}

	Loader::ProcessQueue();

	TextureLoader::ProcessQueue();

	UpdateNexusLink();

	if (hook)
	{
		hook = false;

		// both the commented out and the other variant work. just different approaches.

		/*// Get the currently bound render targets
		ID3D11RenderTargetView* pRTVs[8] = { nullptr }; // Adjust the array size based on your needs
		ID3D11DepthStencilView* pDSV = nullptr; // Optional, depending on your setup
		Renderer::DeviceContext->OMGetRenderTargets(8, pRTVs, &pDSV);

		// Assuming you have one render target
		ID3D11RenderTargetView* pCurrentRTV = pRTVs[0];

		if (pCurrentRTV)
		{
			// Get the texture associated with the render target view
			ID3D11Resource* pRenderTargetResource = nullptr;
			pCurrentRTV->GetResource(&pRenderTargetResource);

			ID3D11Texture2D* pTexture = nullptr;
			HRESULT hr = pRenderTargetResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
			if (SUCCEEDED(hr))
			{
				D3D11_TEXTURE2D_DESC desc;
				pTexture->GetDesc(&desc);

				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;

				// Create a new texture to copy the data from the original texture
				ID3D11Texture2D* pNewTexture = nullptr;
				hr = Renderer::Device->CreateTexture2D(&desc, nullptr, &pNewTexture);
				if (SUCCEEDED(hr) && pNewTexture) {
					// Copy data from the original texture to the new texture
					Renderer::DeviceContext->CopyResource(pNewTexture, pTexture);

					Texture* tex = new Texture();
					tex->Width = Renderer::Width;
					tex->Height = Renderer::Height;

					// Assuming you want to create a ShaderResourceView for the captured texture
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Adjust based on your render target format
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = 1;
					srvDesc.Texture2D.MostDetailedMip = 0;

					if (SUCCEEDED(Renderer::Device->CreateShaderResourceView(pNewTexture, &srvDesc, &tex->Resource)))
					{
						TextureLoader::Mutex.lock();
						TextureLoader::Registry["meme"] = tex;
						TextureLoader::Mutex.unlock();
					}

					pNewTexture->Release();
				}

				pTexture->Release();
			}

			// Release the render target resource
			pRenderTargetResource->Release();
		}

		// Release the retrieved render targets and depth stencil view
		for (int i = 0; i < 8; ++i)
		{
			if (pRTVs[i])
				pRTVs[i]->Release();
		}

		if (pDSV)
			pDSV->Release();*/


		ID3D11Texture2D* pBackBuffer = nullptr;
		if (SUCCEEDED(Renderer::SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
		{
			D3D11_TEXTURE2D_DESC desc;
			pBackBuffer->GetDesc(&desc);

			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;

			ID3D11Texture2D* pStagingTexture;
			if (SUCCEEDED(Renderer::Device->CreateTexture2D(&desc, nullptr, &pStagingTexture)))
			{
				Renderer::DeviceContext->CopyResource(pStagingTexture, pBackBuffer);

				Texture* tex = new Texture();
				tex->Width = Renderer::Width;
				tex->Height = Renderer::Height;

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				ZeroMemory(&srvDesc, sizeof(srvDesc));
				srvDesc.Format = desc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = desc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;

				HRESULT hr = Renderer::Device->CreateShaderResourceView(pStagingTexture, &srvDesc, &tex->Resource);
				if (SUCCEEDED(hr))
				{
					TextureLoader::Mutex.lock();
					TextureLoader::Registry["pre"] = tex;
					TextureLoader::Mutex.unlock();
				}

				pStagingTexture->Release();
			}

			pBackBuffer->Release();
		}
	}

	GUI::Render();

	return Hooks::DXGI_Present(pChain, SyncInterval, Flags);
}
HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	GUI::Shutdown();

	/* Cache window dimensions */
	Renderer::Width = Width;
	Renderer::Height = Height;

	/* Already write to nexus link, as addons depend on that and the next frame isn't called yet so no update to values*/
	NexusLink->Width = Width;
	NexusLink->Height = Height;

	Events::Raise(EV_WINDOW_RESIZED, nullptr);
	
	return Hooks::DXGI_ResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

/* dx */
bool DxLoad()
{
	if (State::Directx < EDxState::LOAD)
	{
		State::Directx = EDxState::LOAD;

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

		State::Directx = EDxState::LOADED;

		if (State::Directx < EDxState::HOOKED && !State::IsVanilla)
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

			State::Directx = EDxState::HOOKED;
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

	if (State::Directx >= EDxState::LOADED)
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

	if (State::Directx >= EDxState::LOADED)
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
}
HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice)
{
	if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_CREATEDEVICE; }

	static decltype(&D3D11CoreCreateDevice) func;
	static const char* func_name = "D3D11CoreCreateDevice";
	Log(CH_CORE, func_name);

	if (State::Directx >= EDxState::LOADED)
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

	if (State::Directx >= EDxState::LOADED)
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

	if (State::Directx >= EDxState::LOADED)
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

	if (State::Directx >= EDxState::LOADED)
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