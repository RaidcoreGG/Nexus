#include "Proxy.h"

#include <filesystem>
#include <string>

#include "Main.h"

#include "Consts.h"
#include "Core.h"
#include "Hooks.h"
#include "Paths.h"
#include "Shared.h"
#include "State.h"

#include "minhook/mh_hook.h"

namespace Proxy
{
	namespace D3D11
	{
		extern PFN_D3D11_CREATE_DEVICE					CreateDevice				= nullptr;
		extern PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN	CreateDeviceAndSwapChain	= nullptr;
		extern PFN_D3D11_CORE_CREATE_DEVICE				CoreCreateDevice			= nullptr;
		extern PFN_D3D11_CORE_CREATE_LAYERED_DEVICE		CoreCreateLayeredDevice		= nullptr;
		extern PFN_D3D11_CORE_GET_LAYERED_DEVICE_SIZE	CoreGetLayeredDeviceSize	= nullptr;
		extern PFN_D3D11_CORE_REGISTER_LAYERS			CoreRegisterLayers			= nullptr;

		bool DxLoad()
		{
			if (State::Directx < EDxState::LOAD)
			{
				State::Directx = EDxState::LOAD;

				/* attempt to chainload */
				/* sanity check that the current dll isn't the chainload */
				if (Path::F_HOST_DLL != Path::F_CHAINLOAD_DLL)
				{
					if (std::filesystem::exists(Path::F_CHAINLOAD_DLL))
					{
						State::IsChainloading = true;

						LogInfo(CH_CORE, "Attempting to chainload.");

						std::string strChainload = Path::F_CHAINLOAD_DLL.string();
						D3D11Handle = LoadLibraryA(strChainload.c_str());
					}
				}

				if (!D3D11Handle)
				{
					if (State::IsChainloading)
					{
						LogWarning(CH_CORE, "Chainload failed to load. Last Error: %u", GetLastError());
						State::IsChainloading = false;
					}

					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11Handle = LoadLibraryA(strSystem.c_str());

					assert(D3D11Handle && "Could not load system d3d11.dll");

					LogInfo(CH_CORE, "Loaded System DLL: %s", strSystem.c_str());
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

						if (SUCCEEDED(Proxy::D3D11::D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swap_desc, &swap, &device, 0, &context)))
						{
							LPVOID* vtbl;

							vtbl = *(LPVOID**)swap;
							//dxgi_release = hook_vtbl_fn(vtbl, 2, dxgi_release_hook);
							//dxgi_present = hook_vtbl_fn(vtbl, 8, dxgi_present_hook);
							//dxgi_resize_buffers = hook_vtbl_fn(vtbl, 13, dxgi_resize_buffers_hook);

							MH_CreateHook(vtbl[8], (LPVOID)&Hooks::DXGIPresent, (LPVOID*)&Hooks::DXGI::Present);
							MH_CreateHook(vtbl[13], (LPVOID)&Hooks::DXGIResizeBuffers, (LPVOID*)&Hooks::DXGI::ResizeBuffers);
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
			if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CREATEDEVICE; Main::Initialize(); }

			if (State::Directx >= EDxState::LOADED && Proxy::D3D11::CreateDevice)
			{
				LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				Proxy::D3D11::CreateDevice = nullptr;

				if (FindFunction(D3D11SystemHandle, &Proxy::D3D11::CreateDevice, "D3D11CreateDevice") == false)
				{
					return 0;
				}
			}

			if (DxLoad() == false) { return 0; }

			if (Proxy::D3D11::CreateDevice == nullptr)
			{
				if (FindFunction(D3D11Handle, &Proxy::D3D11::CreateDevice, "D3D11CreateDevice") == false)
				{
					return 0;
				}
			}

			return Proxy::D3D11::CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
		}
		HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
		{
			if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CREATEDEVICEANDSWAPCHAIN; Main::Initialize(); }

			if (State::Directx >= EDxState::LOADED && Proxy::D3D11::CreateDeviceAndSwapChain)
			{
				LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				Proxy::D3D11::CreateDeviceAndSwapChain = nullptr;

				if (FindFunction(D3D11SystemHandle, &Proxy::D3D11::CreateDeviceAndSwapChain, "D3D11CreateDeviceAndSwapChain") == false)
				{
					return 0;
				}
			}

			if (DxLoad() == false) { return 0; }

			if (Proxy::D3D11::CreateDeviceAndSwapChain == nullptr)
			{
				if (FindFunction(D3D11Handle, &Proxy::D3D11::CreateDeviceAndSwapChain, "D3D11CreateDeviceAndSwapChain") == false)
				{
					return 0;
				}
			}

			return Proxy::D3D11::CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
		}
		HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice)
		{
			if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_CREATEDEVICE; Main::Initialize(); }

			if (State::Directx >= EDxState::LOADED && Proxy::D3D11::CoreCreateDevice)
			{
				LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				Proxy::D3D11::CoreCreateDevice = nullptr;

				if (FindFunction(D3D11SystemHandle, &Proxy::D3D11::CoreCreateDevice, "D3D11CoreCreateDevice") == false)
				{
					return 0;
				}
			}

			if (DxLoad() == false) { return 0; }

			if (Proxy::D3D11::CoreCreateDevice == nullptr)
			{
				if (FindFunction(D3D11Handle, &Proxy::D3D11::CoreCreateDevice, "D3D11CoreCreateDevice") == false)
				{
					return 0;
				}
			}

			return Proxy::D3D11::CoreCreateDevice(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
		}
		HRESULT __stdcall D3D11CoreCreateLayeredDevice(const void* unknown0, DWORD unknown1, const void* unknown2, REFIID riid, void** ppvObj)
		{
			if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_CREATELAYEREDDEVICE; Main::Initialize(); }

			if (State::Directx >= EDxState::LOADED && Proxy::D3D11::CoreCreateLayeredDevice)
			{
				LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				Proxy::D3D11::CoreCreateLayeredDevice = nullptr;

				if (FindFunction(D3D11SystemHandle, &Proxy::D3D11::CoreCreateLayeredDevice, "D3D11CoreCreateLayeredDevice") == false)
				{
					return 0;
				}
			}

			if (DxLoad() == false) { return 0; }

			if (Proxy::D3D11::CoreCreateLayeredDevice == nullptr)
			{
				if (FindFunction(D3D11Handle, &Proxy::D3D11::CoreCreateLayeredDevice, "D3D11CoreCreateLayeredDevice") == false)
				{
					return 0;
				}
			}

			return Proxy::D3D11::CoreCreateLayeredDevice(unknown0, unknown1, unknown2, riid, ppvObj);
		}
		SIZE_T  __stdcall D3D11CoreGetLayeredDeviceSize(const void* unknown0, DWORD unknown1)
		{
			if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_GETLAYEREDDEVICESIZE; Main::Initialize(); }

			if (State::Directx >= EDxState::LOADED && Proxy::D3D11::CoreGetLayeredDeviceSize)
			{
				LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				Proxy::D3D11::CoreGetLayeredDeviceSize = nullptr;

				if (FindFunction(D3D11SystemHandle, &Proxy::D3D11::CoreGetLayeredDeviceSize, "D3D11CoreGetLayeredDeviceSize") == false)
				{
					return 0;
				}
			}

			if (DxLoad() == false) { return 0; }

			if (Proxy::D3D11::CoreGetLayeredDeviceSize == nullptr)
			{
				if (FindFunction(D3D11Handle, &Proxy::D3D11::CoreGetLayeredDeviceSize, "D3D11CoreGetLayeredDeviceSize") == false)
				{
					return 0;
				}
			}

			return Proxy::D3D11::CoreGetLayeredDeviceSize(unknown0, unknown1);
		}
		HRESULT __stdcall D3D11CoreRegisterLayers(const void* unknown0, DWORD unknown1)
		{
			if (State::EntryMethod == EEntryMethod::NONE) { State::EntryMethod = EEntryMethod::CORE_REGISTERLAYERS; Main::Initialize(); }

			if (State::Directx >= EDxState::LOADED && Proxy::D3D11::CoreRegisterLayers)
			{
				LogWarning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Path::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				Proxy::D3D11::CoreRegisterLayers = nullptr;

				if (FindFunction(D3D11SystemHandle, &Proxy::D3D11::CoreRegisterLayers, "D3D11CoreRegisterLayers") == false)
				{
					return 0;
				}
			}

			if (DxLoad() == false) { return 0; }

			if (Proxy::D3D11::CoreRegisterLayers == nullptr)
			{
				if (FindFunction(D3D11Handle, &Proxy::D3D11::CoreRegisterLayers, "D3D11CoreRegisterLayers") == false)
				{
					return 0;
				}
			}

			return Proxy::D3D11::CoreRegisterLayers(unknown0, unknown1);
		}
	}
}
