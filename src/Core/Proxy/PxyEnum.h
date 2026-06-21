///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyEnum.h
/// Description  :  Enumerations for proxy functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EProxyFunction Enumeration
///----------------------------------------------------------------------------------------------------
enum class EProxyFunction : uint32_t
{
	NONE,

	/* D3D11 */
	D3D11_CREATEDEVICE,
	D3D11_CREATEDEVICEANDSWAPCHAIN,
	D3D11_CORECREATEDEVICE,
	D3D11_CORECREATELAYEREDDEVICE,
	D3D11_COREGETLAYEREDDEVICESIZE,
	D3D11_COREREGISTERLAYERS,

	/* DXGI */
	DXGI_CreateDXGIFactory,
	DXGI_CreateDXGIFactory1,
	DXGI_CreateDXGIFactory2,
	DXGI_DXGIGetDebugInterface1,
	DXGI_CompatValue,
	DXGI_CompatString
};
