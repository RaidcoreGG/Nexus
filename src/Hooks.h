#ifndef HOOKS_H
#define HOOKS_H

#include <Windows.h>
#include <d3d11.h>

typedef HRESULT(__stdcall* DXPRESENT)		(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* DXRESIZEBUFFERS)	(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

/* Namespace for hooked functions */
namespace Hooks
{
	extern DXPRESENT		DXGI_Present;
	extern DXRESIZEBUFFERS	DXGI_ResizeBuffers;
	extern WNDPROC			GW2_WndProc;
}

#endif