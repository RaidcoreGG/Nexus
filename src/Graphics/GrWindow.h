///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GrWindow.h
/// Description  :  Definition for the renderer window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <d3d11.h>
#include <dxgi.h>

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::Graphics Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::Graphics
{
	///----------------------------------------------------------------------------------------------------
	/// Window_t Struct
	///----------------------------------------------------------------------------------------------------
	struct Window_t
	{
		IDXGISwapChain*         SwapChain     = nullptr;
		ID3D11Device*           Device        = nullptr;
		ID3D11DeviceContext*    DeviceContext = nullptr;
		ID3D11RenderTargetView* RenderTarget  = nullptr;
		uint32_t                Width         = 0;
		uint32_t                Height        = 0;
	};
}
