#include "Renderer.h"

namespace Renderer
{
	ID3D11Device*			Device				= 0;
	ID3D11DeviceContext*	DeviceContext		= 0;
	IDXGISwapChain*			SwapChain			= 0;
	ID3D11RenderTargetView* RenderTargetView	= 0;
	ImGuiContext*			GuiContext			= 0;

	HWND					WindowHandle		= nullptr;

	unsigned				Width				= 0;
	unsigned				Height				= 0;
	float					Scaling				= 1.00f;
}