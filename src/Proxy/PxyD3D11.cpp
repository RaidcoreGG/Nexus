///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyD3D11.cpp
/// Description  :  Implementation of proxy functions for D3D11.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PxyD3D11.h"

#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <windows.h>

#include "Core/Main.h"
#include "Proxy.h"
#include "PxyEnum.h"
#include "Util/CmdLine.h"

static ProxyModule_t s_ProxyModule{};

#define PROXY_MODULE_NAME "d3d11.dll"

PROXY HRESULT __stdcall D3D11CreateDevice(
	IDXGIAdapter*            pAdapter,
	D3D_DRIVER_TYPE          DriverType,
	HMODULE                  Software,
	UINT                     Flags,
	const D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT                     FeatureLevels,
	UINT                     SDKVersion,
	ID3D11Device**           ppDevice,
	D3D_FEATURE_LEVEL*       pFeatureLevel,
	ID3D11DeviceContext**    ppImmediateContext
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Main::Initialize(EProxyFunction::D3D11_CREATEDEVICE);

	return D3D11CreateDeviceAndSwapChain(
		pAdapter,
		DriverType,
		Software,
		Flags,
		pFeatureLevels,
		FeatureLevels,
		SDKVersion,
		nullptr,
		nullptr,
		ppDevice,
		pFeatureLevel,
		ppImmediateContext
	);
}

PROXY HRESULT __stdcall D3D11CreateDeviceAndSwapChain(
	IDXGIAdapter*               pAdapter,
	D3D_DRIVER_TYPE             DriverType,
	HMODULE                     Software,
	UINT                        Flags,
	const D3D_FEATURE_LEVEL*    pFeatureLevels,
	UINT                        FeatureLevels,
	UINT                        SDKVersion,
	const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
	IDXGISwapChain**            ppSwapChain,
	ID3D11Device**              ppDevice,
	D3D_FEATURE_LEVEL*          pFeatureLevel,
	ID3D11DeviceContext**       ppImmediateContext
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Main::Initialize(EProxyFunction::D3D11_CREATEDEVICEANDSWAPCHAIN);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3D11CreateDeviceAndSwapChain)>("D3D11CreateDeviceAndSwapChain", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	if (CmdLine::HasArgument("-ggdev"))
	{
		Flags |= D3D11_CREATE_DEVICE_DEBUG;
	}

	return fn(
		pAdapter,
		DriverType,
		Software,
		Flags,
		pFeatureLevels,
		FeatureLevels,
		SDKVersion,
		pSwapChainDesc,
		ppSwapChain,
		ppDevice,
		pFeatureLevel,
		ppImmediateContext
	);
}

PROXY HRESULT __stdcall D3D11CoreCreateDevice(
	IDXGIFactory*            pFactory,
	IDXGIAdapter*            pAdapter,
	UINT                     Flags,
	const D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT                     FeatureLevels,
	ID3D11Device**           ppDevice
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Main::Initialize(EProxyFunction::D3D11_CORECREATEDEVICE);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3D11CoreCreateDevice)>("D3D11CoreCreateDevice", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		pFactory,
		pAdapter,
		Flags,
		pFeatureLevels,
		FeatureLevels,
		ppDevice
	);
}

PROXY HRESULT __stdcall D3D11CoreCreateLayeredDevice(
	const void* unknown0,
	DWORD       unknown1,
	const void* unknown2,
	REFIID      riid,
	void**      ppvObj
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Main::Initialize(EProxyFunction::D3D11_CORECREATELAYEREDDEVICE);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3D11CoreCreateLayeredDevice)>("D3D11CoreCreateLayeredDevice", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		unknown0,
		unknown1,
		unknown2,
		riid,
		ppvObj
	);
}

PROXY SIZE_T __stdcall D3D11CoreGetLayeredDeviceSize(
	const void* unknown0,
	DWORD       unknown1
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Main::Initialize(EProxyFunction::D3D11_COREGETLAYEREDDEVICESIZE);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3D11CoreGetLayeredDeviceSize)>("D3D11CoreGetLayeredDeviceSize", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		unknown0,
		unknown1
	);
}

PROXY HRESULT __stdcall D3D11CoreRegisterLayers(
	const void* unknown0,
	DWORD       unknown1
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Main::Initialize(EProxyFunction::D3D11_COREREGISTERLAYERS);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3D11CoreRegisterLayers)>("D3D11CoreRegisterLayers", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		unknown0,
		unknown1
	);
}
