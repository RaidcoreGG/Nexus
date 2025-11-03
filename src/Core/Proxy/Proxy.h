///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Proxy.h
/// Description  :  Implementation of proxy functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <d3dcommon.h>
#include <dxgi.h>

#define PROXY extern "C" __declspec(dllexport)

struct ID3D11Device;
struct ID3D11DeviceContext;

///----------------------------------------------------------------------------------------------------
/// (PROXY) D3D11CreateDevice
///----------------------------------------------------------------------------------------------------
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
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) D3D11CreateDeviceAndSwapChain
///----------------------------------------------------------------------------------------------------
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
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) D3D11CoreCreateDevice
///----------------------------------------------------------------------------------------------------
PROXY HRESULT __stdcall D3D11CoreCreateDevice(
	IDXGIFactory*            pFactory,
	IDXGIAdapter*            pAdapter,
	UINT                     Flags,
	const D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT                     FeatureLevels,
	ID3D11Device**           ppDevice
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) D3D11CoreCreateLayeredDevice
///----------------------------------------------------------------------------------------------------
PROXY HRESULT __stdcall D3D11CoreCreateLayeredDevice(
	const void* unknown0,
	DWORD       unknown1,
	const void* unknown2,
	REFIID      riid,
	void**      ppvObj
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) D3D11CoreGetLayeredDeviceSize
///----------------------------------------------------------------------------------------------------
PROXY SIZE_T __stdcall D3D11CoreGetLayeredDeviceSize(
	const void* unknown0,
	DWORD       unknown1
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) D3D11CoreRegisterLayers
///----------------------------------------------------------------------------------------------------
PROXY HRESULT __stdcall D3D11CoreRegisterLayers(
	const void* unknown0,
	DWORD       unknown1
);
