#include "Renderer.h"

namespace Renderer
{
	ID3D11Device*			Device				= nullptr;
	ID3D11DeviceContext*	DeviceContext		= nullptr;
	IDXGISwapChain*			SwapChain			= nullptr;
	ID3D11RenderTargetView* RenderTargetView	= nullptr;
	ImGuiContext*			GuiContext			= nullptr;

	HWND					WindowHandle		= nullptr;

	unsigned				Width				= 0;
	unsigned				Height				= 0;
	float					Scaling				= 0;
}