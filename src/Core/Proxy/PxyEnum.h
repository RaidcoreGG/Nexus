///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyEnum.h
/// Description  :  Enumerations for proxy functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef PXYENUM_H
#define PXYENUM_H

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
	/* TODO */
};

#endif
