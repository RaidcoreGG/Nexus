///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RdrContext.h
/// Description  :  Definition for the renderer context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef RDRCONTEXT_H
#define RDRCONTEXT_H

#include <dxgi.h>
#include <d3d11.h>

#include "RdrWindow.h"
#include "RdrMetrics.h"

///----------------------------------------------------------------------------------------------------
/// RenderContext_t Struct
///----------------------------------------------------------------------------------------------------
struct RenderContext_t
{
	ID3D11Device*        Device        = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	IDXGISwapChain*      SwapChain     = nullptr;

	RenderWindow_t       Window        = {};

	RenderMetrics_t      Metrics       = {};
};

#endif
