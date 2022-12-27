#ifndef RENDERER_H
#define RENDERER_H

#include <d3d11.h>

namespace Renderer
{
	extern ID3D11Device*			Device;
	extern ID3D11DeviceContext*		DeviceContext;
	extern IDXGISwapChain*			SwapChain;
	extern ID3D11RenderTargetView*	RenderTargetView;
}

#endif