#ifndef PROXY_H
#define PROXY_H

#include <d3d11.h>
#include <Windows.h>

///----------------------------------------------------------------------------------------------------
/// EEntryMethod Enumeration
///----------------------------------------------------------------------------------------------------
enum class EEntryMethod
{
	NONE,
	CREATEDEVICE,
	CREATEDEVICEANDSWAPCHAIN,
	CORE_CREATEDEVICE,
	CORE_CREATELAYEREDDEVICE,
	CORE_GETLAYEREDDEVICESIZE,
	CORE_REGISTERLAYERS
};

///----------------------------------------------------------------------------------------------------
/// Proxy Namespace
///----------------------------------------------------------------------------------------------------
namespace Proxy
{
	HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
	HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
	HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device** ppDevice);
	HRESULT __stdcall D3D11CoreCreateLayeredDevice(const void* unknown0, DWORD unknown1, const void* unknown2, REFIID riid, void** ppvObj);
	SIZE_T  __stdcall D3D11CoreGetLayeredDeviceSize(const void* unknown0, DWORD unknown1);
	HRESULT __stdcall D3D11CoreRegisterLayers(const void* unknown0, DWORD unknown1);
}

#endif
