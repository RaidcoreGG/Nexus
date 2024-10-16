#ifndef HOOKS_H
#define HOOKS_H

#include <dxgi.h>
#include <Windows.h>

#include "Events/EventHandler.h"
#include "UI/UiContext.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"
#include "Loader/NexusLinkData.h"
#include "Services/Textures/TextureLoader.h"
#include "Inputs/RawInput/RawInputApi.h"

typedef HRESULT (__stdcall*DXPRESENT)		(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (__stdcall*DXRESIZEBUFFERS)	(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

/* Namespace for hooked functions */
namespace Hooks
{
	extern NexusLinkData*  NexusLink;
	extern CRawInputApi*   RawInputApi;
	extern CInputBindApi*  InputBindApi;
	extern CUiContext*     UIContext;
	extern CTextureLoader* TextureService;
	extern CEventApi*      EventApi;

	namespace DXGI
	{
		extern DXPRESENT       Present;
		extern DXRESIZEBUFFERS ResizeBuffers;
	}

	namespace GW2
	{
		extern WNDPROC         WndProc;
	}

	LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT __stdcall DXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
	HRESULT __stdcall DXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
}

#endif
