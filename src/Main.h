#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>

#include "Proxy.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

namespace Hooks
{
	typedef HRESULT(__stdcall* DXPRESENT)       (IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
	typedef HRESULT(__stdcall* DXRESIZEBUFFERS) (IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

	extern DXPRESENT       DXGIPresent;
	extern DXRESIZEBUFFERS DXGIResizeBuffers;
	extern WNDPROC         WndProc;
}

namespace Main
{
	void Initialize(EEntryMethod aEntryMethod);

	void Shutdown(unsigned int aReason);

	LRESULT __stdcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);

	HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
}

#endif
