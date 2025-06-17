///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RiApi.cpp
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RiApi.h"

#include <assert.h>

CRawInputApi::CRawInputApi(RenderContext_t* aRenderCtx)
{
	assert(aRenderCtx);

	this->RenderContext = aRenderCtx;
}

UINT CRawInputApi::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// don't pass to game if addon wndproc
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (WNDPROC_CALLBACK wndprocCb : this->Registry)
	{
		if (wndprocCb(hWnd, uMsg, wParam, lParam) == 0)
		{
			return 0;
		}
	}

	return 1;
}

UINT CRawInputApi::WndProcGameOnly(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* offset of 7997, if uMsg in that range it's a nexus game only message */
	if (uMsg >= WM_PASSTHROUGH_FIRST && uMsg <= WM_PASSTHROUGH_LAST)
	{
		/* modify the uMsg code to the original code */
		uMsg -= WM_PASSTHROUGH_FIRST;
	}

	return uMsg;
}

LRESULT CRawInputApi::SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg < WM_USER)
	{
		return PostMessageA(this->RenderContext->Window.Handle, uMsg + WM_PASSTHROUGH_FIRST, wParam, lParam);
	}

	return PostMessageA(this->RenderContext->Window.Handle, uMsg, wParam, lParam);
}

void CRawInputApi::Register(WNDPROC_CALLBACK aWndProcCallback)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.push_back(aWndProcCallback);
}

void CRawInputApi::Deregister(WNDPROC_CALLBACK aWndProcCallback)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.erase(std::remove(this->Registry.begin(), this->Registry.end(), aWndProcCallback), this->Registry.end());
}

int CRawInputApi::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	for (WNDPROC_CALLBACK wndprocCb : this->Registry)
	{
		if (wndprocCb >= aStartAddress && wndprocCb <= aEndAddress)
		{
			this->Registry.erase(std::remove(this->Registry.begin(), this->Registry.end(), wndprocCb), this->Registry.end());
			refCounter++;
		}
	}

	return refCounter;
}
