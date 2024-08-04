#include "Proxy.h"

#include <filesystem>
#include <string>

#include "Main.h"

#include "Consts.h"
#include "Hooks.h"
#include "Index.h"
#include "Shared.h"
#include "State.h"

#include "Util/DLL.h"
#include "Util/MD5.h"
#include "Util/Memory.h"

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

		///----------------------------------------------------------------------------------------------------
		/// DxLoad:
		/// 	Loads the target DirectX DLL and hooks the functions necessary to render.
		///----------------------------------------------------------------------------------------------------
		bool DxLoad()
		{
			if (State::Directx < EDxState::LOAD)
			{
				State::Directx = EDxState::LOAD;

				/* attempt to chainload */
				/* sanity check that the current dll isn't the chainload */
				if (Index::F_HOST_DLL != Index::F_CHAINLOAD_DLL)
				{
					if (std::filesystem::exists(Index::F_CHAINLOAD_DLL))
					{
						if (MD5Util::FromFile(Index::F_HOST_DLL) == MD5Util::FromFile(Index::F_CHAINLOAD_DLL))
						{
							try
							{
								std::filesystem::remove(Index::F_CHAINLOAD_DLL);

								Logger->Info(CH_LOADER, "Removed duplicate Nexus from chainload.");
							}
							catch (std::filesystem::filesystem_error fErr)
							{
								Logger->Debug(CH_LOADER, "%s", fErr.what());
							}
						}
						else
						{
							State::IsChainloading = true;

							std::string strChainload = Index::F_CHAINLOAD_DLL.string();
							D3D11Handle = LoadLibraryA(strChainload.c_str());

							if (D3D11Handle)
							{
								Logger->Info(CH_CORE, "Loaded Chainload DLL: %s", strChainload.c_str());
							}
						}
					}
				}

				if (!D3D11Handle)
				{
					if (State::IsChainloading)
					{
						Logger->Warning(CH_CORE, "Chainload failed to load. Last Error: %u", GetLastError());
						State::IsChainloading = false;
					}

					std::string strSystem = Index::F_SYSTEM_DLL.string();
					D3D11Handle = LoadLibraryA(strSystem.c_str());

					assert(D3D11Handle && "Could not load system d3d11.dll");

					Logger->Info(CH_CORE, "Loaded System DLL: %s", strSystem.c_str());
				}

				State::Directx = EDxState::LOADED;

				if (State::Directx < EDxState::HOOKED && !State::IsVanilla)
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

						if (SUCCEEDED(Proxy::D3D11::D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swap_desc, &swap, &device, 0, &context)))
						{
							LPVOID* vtbl;

							vtbl = *(LPVOID**)swap;
							//dxgi_release = hook_vtbl_fn(vtbl, 2, dxgi_release_hook);
							//dxgi_present = hook_vtbl_fn(vtbl, 8, dxgi_present_hook);
							//dxgi_resize_buffers = hook_vtbl_fn(vtbl, 13, dxgi_resize_buffers_hook);

							MH_CreateHook(Memory::FollowJmpChain((PBYTE)vtbl[8]), (LPVOID)&Hooks::DXGIPresent, (LPVOID*)&Hooks::DXGI::Present);
							MH_CreateHook(Memory::FollowJmpChain((PBYTE)vtbl[13]), (LPVOID)&Hooks::DXGIResizeBuffers, (LPVOID*)&Hooks::DXGI::ResizeBuffers);
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

			if (!D3D11Handle)
			{
				Logger->Critical(CH_CORE, "Could not acquire D3D11 handle.");
				return false;
			}

			return true;
		}

		///----------------------------------------------------------------------------------------------------
		/// InitProxyFunc:
		/// 	Loads the proxy function and returns true on success, false on error.
		///----------------------------------------------------------------------------------------------------
		bool InitProxyFunc(EEntryMethod aEntryMethod, void** aFunction, const char* aName)
		{
			/* Set entry method and call Nexus entry. */
			if (State::EntryMethod == EEntryMethod::NONE)
			{
				State::EntryMethod = aEntryMethod;
				Main::Initialize();
			}

			/* Check if this is a rebound. */
			if (State::Directx >= EDxState::LOADED && *aFunction)
			{
				Logger->Warning(CH_CORE, "DirectX entry already called. Chainload bounced back. Redirecting to system D3D11.");

				if (!D3D11SystemHandle)
				{
					std::string strSystem = Index::F_SYSTEM_DLL.string();
					D3D11SystemHandle = LoadLibraryA(strSystem.c_str());
				}

				*aFunction = nullptr;

				if (DLL::FindFunction(D3D11SystemHandle, aFunction, aName) == false)
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
				if (DLL::FindFunction(D3D11Handle, aFunction, aName) == false)
				{
					return false;
				}
			}

			return true;
		}

		HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
		{
			if (InitProxyFunc(EEntryMethod::CREATEDEVICE, (void**)&Proxy::D3D11::CreateDevice, "D3D11CreateDevice") == false)
			{
				return 0;
			}

			return Proxy::D3D11::CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
		}
		HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
		{
			if (InitProxyFunc(EEntryMethod::CREATEDEVICEANDSWAPCHAIN, (void**)&Proxy::D3D11::CreateDeviceAndSwapChain, "D3D11CreateDeviceAndSwapChain") == false)
			{
				return 0;
			}

			return Proxy::D3D11::CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
		}
		HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice)
		{
			if (InitProxyFunc(EEntryMethod::CORE_CREATEDEVICE, (void**)&Proxy::D3D11::CoreCreateDevice, "D3D11CoreCreateDevice") == false)
			{
				return 0;
			}

			return Proxy::D3D11::CoreCreateDevice(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
		}
		HRESULT __stdcall D3D11CoreCreateLayeredDevice(const void* unknown0, DWORD unknown1, const void* unknown2, REFIID riid, void** ppvObj)
		{
			if (InitProxyFunc(EEntryMethod::CORE_CREATELAYEREDDEVICE, (void**)&Proxy::D3D11::CoreCreateLayeredDevice, "D3D11CoreCreateLayeredDevice") == false)
			{
				return 0;
			}

			return Proxy::D3D11::CoreCreateLayeredDevice(unknown0, unknown1, unknown2, riid, ppvObj);
		}
		SIZE_T  __stdcall D3D11CoreGetLayeredDeviceSize(const void* unknown0, DWORD unknown1)
		{
			if (InitProxyFunc(EEntryMethod::CORE_GETLAYEREDDEVICESIZE, (void**)&Proxy::D3D11::CoreGetLayeredDeviceSize, "D3D11CoreGetLayeredDeviceSize") == false)
			{
				return 0;
			}
			
			return Proxy::D3D11::CoreGetLayeredDeviceSize(unknown0, unknown1);
		}
		HRESULT __stdcall D3D11CoreRegisterLayers(const void* unknown0, DWORD unknown1)
		{
			if (InitProxyFunc(EEntryMethod::CORE_REGISTERLAYERS, (void**)&Proxy::D3D11::CoreRegisterLayers, "D3D11CoreRegisterLayers") == false)
			{
				return 0;
			}

			return Proxy::D3D11::CoreRegisterLayers(unknown0, unknown1);
		}
	}
}
