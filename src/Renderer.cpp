#include "Renderer.h"

namespace Renderer
{
	ID3D11Device*			Device				= nullptr;
	ID3D11DeviceContext*	DeviceContext		= nullptr;
	IDXGISwapChain*			SwapChain			= nullptr;

	HWND					WindowHandle		= nullptr;

	long long				FrameCounter		= 0;

	unsigned				Width				= 0;
	unsigned				Height				= 0;
	float					Scaling				= 0;
}