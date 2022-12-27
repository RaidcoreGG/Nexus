#include "Renderer.h"

namespace Renderer
{
	ID3D11Device*			Device;
	ID3D11DeviceContext*	DeviceContext;
	IDXGISwapChain*			SwapChain;
	ID3D11RenderTargetView* RenderTargetView;
}