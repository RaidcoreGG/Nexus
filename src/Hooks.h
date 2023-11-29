#ifndef HOOKS_H
#define HOOKS_H

#include <Windows.h>
#include <dxgi.h>

typedef HRESULT (__stdcall*DXPRESENT)		(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (__stdcall*DXRESIZEBUFFERS)	(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

/* Namespace for hooked functions */
namespace Hooks
{
	extern DXPRESENT		DXGI_Present;
	extern DXRESIZEBUFFERS	DXGI_ResizeBuffers;
	extern WNDPROC			GW2_WndProc;
}

#endif