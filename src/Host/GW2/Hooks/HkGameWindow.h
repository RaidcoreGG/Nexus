///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HkGameWindow.h
/// Description  :  Tracks the game's swap chain and window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <dxgi.h>
#include <windows.h>

#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/RawInput/WndProcNotify.h"
#include "Engine/Loader/Loader.h"
#include "HkNotify.h"
#include "UI/Renderer/RdrContext.h"

///----------------------------------------------------------------------------------------------------
/// CGameWindow Class
///----------------------------------------------------------------------------------------------------
class CGameWindow : public IDxgiPresentNotifyHandler, public IDxgiResizeNotifyHandler, public IWndProcNotifyHandler
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CGameWindow(RenderContext_t* aRenderContext, CLoader* aLoader, CDataLinkApi* aDataLink, CEventApi* aEventApi);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CGameWindow();

	///----------------------------------------------------------------------------------------------------
	/// OnDxgiPresentNotify:
	/// 	Counts the frame and takes over the window procedure once the game's swap chain is known.
	///----------------------------------------------------------------------------------------------------
	void OnDxgiPresentNotify(IDXGISwapChain* aSwapChain) override;

	///----------------------------------------------------------------------------------------------------
	/// OnDxgiResizeNotify:
	/// 	Stores the new window size and raises EV_WINDOW_RESIZED.
	///----------------------------------------------------------------------------------------------------
	void OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight) override;

	///----------------------------------------------------------------------------------------------------
	/// OnWndProcNotify:
	/// 	Shuts down on WM_DESTROY. Never consumes.
	///----------------------------------------------------------------------------------------------------
	UINT OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam) override;

	private:
	RenderContext_t* RenderContext;
	CLoader*         Loader;
	CDataLinkApi*    DataLink;
	CEventApi*       EventApi;
};
