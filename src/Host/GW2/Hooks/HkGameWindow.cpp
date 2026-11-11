///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HkGameWindow.cpp
/// Description  :  Tracks the game's swap chain and window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "HkGameWindow.h"

#include <cassert>

#include "Host/GW2/Addons/API/NexusLink.h"
#include "Host/GW2/Main.h"
#include "HkConst.h"
#include "Hooks.h"

CGameWindow::CGameWindow(RenderContext_t* aRenderContext, CLoader* aLoader, CDataLinkApi* aDataLink, CEventApi* aEventApi)
{
	assert(aRenderContext);
	assert(aLoader);
	assert(aDataLink);
	assert(aEventApi);

	this->RenderContext = aRenderContext;
	this->Loader        = aLoader;
	this->DataLink      = aDataLink;
	this->EventApi      = aEventApi;

	this->INotifyHandler<IDxgiPresentNotifyHandler>::Attach(DXGIPRESENT_PRIO_WINDOW);
	this->INotifyHandler<IDxgiResizeNotifyHandler>::Attach(DXGIRESIZE_PRIO_WINDOW);
	this->INotifyHandler<IWndProcNotifyHandler>::Attach(WNDPROC_PRIO_SHUTDOWN);
}

CGameWindow::~CGameWindow()
{
	this->INotifyHandler<IWndProcNotifyHandler>::Detach();
	this->INotifyHandler<IDxgiResizeNotifyHandler>::Detach();
	this->INotifyHandler<IDxgiPresentNotifyHandler>::Detach();
}

void CGameWindow::OnDxgiPresentNotify(IDXGISwapChain* aSwapChain)
{
	this->RenderContext->Metrics.FrameCount++;

	/* The swap chain used to hook is not the one the game created. */
	if (this->RenderContext->SwapChain != aSwapChain)
	{
		this->RenderContext->SwapChain = aSwapChain;

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		this->RenderContext->SwapChain->GetDesc(&swapChainDesc);

		this->RenderContext->Window.Handle = swapChainDesc.OutputWindow;
		Hooks::Target::WndProc = (WNDPROC)SetWindowLongPtr(this->RenderContext->Window.Handle, GWLP_WNDPROC, (LONG_PTR)Hooks::Detour::WndProc);

		this->Loader->InitDirectoryUpdates(this->RenderContext->Window.Handle);
	}
}

void CGameWindow::OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight)
{
	this->RenderContext->Window.Width  = aWidth;
	this->RenderContext->Window.Height = aHeight;

	NexusLinkData_t* nexuslink = (NexusLinkData_t*)this->DataLink->GetResource(DL_NEXUS_LINK);

	/* Written now, as addons read it on the resize event before the next frame updates it. */
	if (nexuslink)
	{
		nexuslink->Width  = aWidth;
		nexuslink->Height = aHeight;
	}

	this->EventApi->Raise(EV_WINDOW_RESIZED);
}

UINT CGameWindow::OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam)
{
	if (uMsg == WM_DESTROY)
	{
		Main::Shutdown(uMsg);
	}

	return 1;
}
