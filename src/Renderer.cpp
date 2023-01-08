#include "Renderer.h"

namespace Renderer
{
	ID3D11Device*			Device				= 0;
	ID3D11DeviceContext*	DeviceContext		= 0;
	IDXGISwapChain*			SwapChain			= 0;
	ID3D11RenderTargetView* RenderTargetView	= 0;

	HWND					WindowHandle		= nullptr;

	unsigned int			Width				= 0;
	unsigned int			Height				= 0;
}