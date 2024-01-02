#ifndef PROXY_H
#define PROXY_H

#include <d3d11.h>
#include <Windows.h>

// CreateDevice defined in d3d11.h
// CreateDeviceAndSwapChain defined in d3d11.h

typedef HRESULT(WINAPI* PFN_D3D11_CORE_CREATE_DEVICE)(IDXGIFactory*, IDXGIAdapter*, UINT, const D3D_FEATURE_LEVEL*, UINT, ID3D11Device**);
typedef HRESULT(WINAPI* PFN_D3D11_CORE_CREATE_LAYERED_DEVICE)(const void*, DWORD, const void*, REFIID, void**);
typedef HRESULT(WINAPI* PFN_D3D11_CORE_GET_LAYERED_DEVICE_SIZE)(const void*, DWORD);
typedef HRESULT(WINAPI* PFN_D3D11_CORE_REGISTER_LAYERS)(const void*, DWORD);

/* Namespace for proxied functions */
namespace Proxy
{
	namespace D3D11
	{
		extern PFN_D3D11_CREATE_DEVICE					CreateDevice;
		extern PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN	CreateDeviceAndSwapChain;
		extern PFN_D3D11_CORE_CREATE_DEVICE				CoreCreateDevice;
		extern PFN_D3D11_CORE_CREATE_LAYERED_DEVICE		CoreCreateLayeredDevice;
		extern PFN_D3D11_CORE_GET_LAYERED_DEVICE_SIZE	CoreGetLayeredDeviceSize;
		extern PFN_D3D11_CORE_REGISTER_LAYERS			CoreRegisterLayers;

		bool DxLoad();
		HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
		HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
		HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice);
		HRESULT __stdcall D3D11CoreCreateLayeredDevice(const void* unknown0, DWORD unknown1, const void* unknown2, REFIID riid, void** ppvObj);
		SIZE_T  __stdcall D3D11CoreGetLayeredDeviceSize(const void* unknown0, DWORD unknown1);
		HRESULT __stdcall D3D11CoreRegisterLayers(const void* unknown0, DWORD unknown1);
	}
}

#endif
