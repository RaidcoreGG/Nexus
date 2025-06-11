///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Proxy.cpp
/// Description  :  Implementation of proxy functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Proxy.h"

#include <filesystem>
#include <string>
#include <d3d11.h>

#include "minhook/mh_hook.h"

#include "Consts.h"
#include "Core/Context.h"
#include "Core/Main.h"
#include "Engine/Index/Index.h"
#include "PxyEnum.h"
#include "PxyFuncDefs.h"
#include "Shared.h"
#include "State.h"
#include "Util/CmdLine.h"
#include "Util/DLL.h"
#include "Util/MD5.h"
#include "Util/Memory.h"
#include "Core/Hooks/Hooks.h"

namespace Proxy
{
	static HMODULE s_D3D11Handle;
	static HMODULE s_D3D11SystemHandle;

	///----------------------------------------------------------------------------------------------------
	/// DxLoad:
	/// 	Loads the target DirectX DLL and hooks the functions necessary to render.
	///----------------------------------------------------------------------------------------------------
	bool DxLoad()
	{
		CContext* ctx = CContext::GetContext();
		CLogApi* logger = ctx->GetLogger();

		if (State::Directx < EDxState::LOAD)
		{
			State::Directx = EDxState::LOAD;

			bool isChainloading = false;

			/* attempt to chainload */
			/* sanity check that the current dll isn't the chainload */
			if (Index(EPath::NexusDLL) != Index(EPath::D3D11Chainload))
			{
				if (std::filesystem::exists(Index(EPath::D3D11Chainload)))
				{
					if (MD5Util::FromFile(Index(EPath::NexusDLL)) == MD5Util::FromFile(Index(EPath::D3D11Chainload)))
					{
						try
						{
							std::filesystem::remove(Index(EPath::D3D11Chainload));

							logger->Info(CH_CORE, "Removed duplicate Nexus from chainload.");
						}
						catch (std::filesystem::filesystem_error fErr)
						{
							logger->Debug(CH_CORE, "%s", fErr.what());
						}
					}
					else
					{
						isChainloading = true;

						std::string strChainload = Index(EPath::D3D11Chainload).string();
						s_D3D11Handle = LoadLibraryA(strChainload.c_str());

						if (s_D3D11Handle)
						{
							logger->Info(CH_CORE, "Loaded Chainload DLL: %s", strChainload.c_str());
						}
					}
				}
			}

			if (!s_D3D11Handle)
			{
				if (isChainloading)
				{
					logger->Warning(CH_CORE, "Chainload failed to load. Last Error: %u", GetLastError());
					isChainloading = false;
				}

				std::string strSystem = Index(EPath::D3D11).string();
				s_D3D11Handle = LoadLibraryA(strSystem.c_str());

				assert(s_D3D11Handle && "Could not load system d3d11.dll");

				logger->Info(CH_CORE, "Loaded System DLL: %s", strSystem.c_str());
			}

			State::Directx = EDxState::LOADED;

			if (State::Directx < EDxState::HOOKED && !CmdLine::HasArgument("-ggvanilla"))
			{
				WNDCLASSEXA wc;
				memset(&wc, 0, sizeof(wc));
				wc.cbSize = sizeof(wc);
				wc.lpfnWndProc = DefWindowProcA;
				wc.hInstance = GetModuleHandleA(0);
				wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
				wc.lpszClassName = "Raidcore_Dx_Window_Class";
				RegisterClassExA(&wc);

				HWND wnd = CreateWindowExA(0, wc.lpszClassName, 0, WS_OVERLAPPED, 0, 0, 1280, 720, 0, 0, wc.hInstance, 0);
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

						MH_CreateHook(Memory::FollowJmpChain((PBYTE)vtbl[8]), (LPVOID)&Hooks::Detour::DXGIPresent, (LPVOID*)&Hooks::Target::DXGIPresent);
						MH_CreateHook(Memory::FollowJmpChain((PBYTE)vtbl[13]), (LPVOID)&Hooks::Detour::DXGIResizeBuffers, (LPVOID*)&Hooks::Target::DXGIResizeBuffers);
						MH_EnableHook(MH_ALL_HOOKS);

						context->Release();
						device->Release();
						swap->Release();
					}

					DestroyWindow(wnd);
				}

				UnregisterClassA(wc.lpszClassName, wc.hInstance);

				State::Directx = EDxState::HOOKED;
			}
		}

		if (!s_D3D11Handle)
		{
			logger->Critical(CH_CORE, "Could not acquire D3D11 handle.");
			return false;
		}

		return true;
	}

	///----------------------------------------------------------------------------------------------------
	/// InitProxyFunc:
	/// 	Loads the proxy function and returns true on success, false on error.
	///----------------------------------------------------------------------------------------------------
	bool InitProxyFunc(void** aFunction, const char* aName)
	{
		CContext* ctx = CContext::GetContext();
		CLogApi* logger = ctx->GetLogger();

		/* Check if this is a rebound. */
		if (State::Directx >= EDxState::LOADED && *aFunction)
		{
			logger->Warning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

			if (!s_D3D11SystemHandle)
			{
				std::string strSystem = Index(EPath::D3D11).string();
				s_D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
			}

			*aFunction = nullptr;

			if (DLL::FindFunction(s_D3D11SystemHandle, aFunction, aName) == false)
			{
				return false;
			}
		}

		/* Get DX Handle. */
		if (DxLoad() == false)
		{
			return false;
		}

		/* Proxy the function. */
		if (*aFunction == nullptr)
		{
			if (DLL::FindFunction(s_D3D11Handle, aFunction, aName) == false)
			{
				return false;
			}
		}

		return true;
	}
}
#pragma warning( push )
#pragma warning(disable: 6101)
PROXY HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	Main::Initialize(EProxyFunction::D3D11_CREATEDEVICE);

	static PFN_D3D11_CREATE_DEVICE s_CreateDevice = nullptr;
	if (Proxy::InitProxyFunc((void**)&s_CreateDevice, "D3D11CreateDevice") == false)
	{
		return 0;
	}

	return s_CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}

PROXY HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	Main::Initialize(EProxyFunction::D3D11_CREATEDEVICEANDSWAPCHAIN);

	static PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN s_CreateDeviceAndSwapChain = nullptr;

	if (Proxy::InitProxyFunc((void**)&s_CreateDeviceAndSwapChain, "D3D11CreateDeviceAndSwapChain") == false)
	{
		return 0;
	}

	return s_CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}

PROXY HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice)
{
	Main::Initialize(EProxyFunction::D3D11_CORECREATEDEVICE);

	static PFN_D3D11_CORE_CREATE_DEVICE s_CoreCreateDevice = nullptr;

	if (Proxy::InitProxyFunc((void**)&s_CoreCreateDevice, "D3D11CoreCreateDevice") == false)
	{
		return 0;
	}

	return s_CoreCreateDevice(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
}

PROXY HRESULT __stdcall D3D11CoreCreateLayeredDevice(const void* unknown0, DWORD unknown1, const void* unknown2, REFIID riid, void** ppvObj)
{
	Main::Initialize(EProxyFunction::D3D11_CORECREATELAYEREDDEVICE);

	static PFN_D3D11_CORE_CREATE_LAYERED_DEVICE s_CoreCreateLayeredDevice = nullptr;

	if (Proxy::InitProxyFunc((void**)&s_CoreCreateLayeredDevice, "D3D11CoreCreateLayeredDevice") == false)
	{
		return 0;
	}

	return s_CoreCreateLayeredDevice(unknown0, unknown1, unknown2, riid, ppvObj);
}

PROXY SIZE_T __stdcall D3D11CoreGetLayeredDeviceSize(const void* unknown0, DWORD unknown1)
{
	Main::Initialize(EProxyFunction::D3D11_COREGETLAYEREDDEVICESIZE);

	static PFN_D3D11_CORE_GET_LAYERED_DEVICE_SIZE s_CoreGetLayeredDeviceSize = nullptr;

	if (Proxy::InitProxyFunc((void**)&s_CoreGetLayeredDeviceSize, "D3D11CoreGetLayeredDeviceSize") == false)
	{
		return 0;
	}

	return s_CoreGetLayeredDeviceSize(unknown0, unknown1);
}

PROXY HRESULT __stdcall D3D11CoreRegisterLayers(const void* unknown0, DWORD unknown1)
{
	Main::Initialize(EProxyFunction::D3D11_COREREGISTERLAYERS);

	static PFN_D3D11_CORE_REGISTER_LAYERS s_CoreRegisterLayers = nullptr;

	if (Proxy::InitProxyFunc((void**)&s_CoreRegisterLayers, "D3D11CoreRegisterLayers") == false)
	{
		return 0;
	}

	return s_CoreRegisterLayers(unknown0, unknown1);
}
#pragma warning( pop )
