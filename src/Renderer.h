#ifndef RENDERER_H
#define RENDERER_H

#include <Windows.h>
#include <d3d11.h>

#include "imgui.h"

/* Namespace for renderer variables */
namespace Renderer
{
	extern ID3D11Device*			Device;
	extern ID3D11DeviceContext*		DeviceContext;
	extern IDXGISwapChain*			SwapChain;
	extern ID3D11RenderTargetView*	RenderTargetView;
	extern ImGuiContext*			GuiContext;

	extern HWND						WindowHandle;		/* GW2's window handle*/

	extern long long				FrameCounter;

	extern unsigned					Width;
	extern unsigned					Height;
	extern float					Scaling;
}

#endif