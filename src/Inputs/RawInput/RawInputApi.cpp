///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RawInputApi.cpp
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RawInputApi.h"

#include "Renderer.h"

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

LRESULT CRawInputApi::SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg >= WM_USER)
	{
		return PostMessageA(Renderer::WindowHandle, uMsg, wParam, lParam);
	}
	else
	{
		return PostMessageA(Renderer::WindowHandle, uMsg + WM_PASSTHROUGH_FIRST, wParam, lParam);
	}

	return PostMessageA(Renderer::WindowHandle, uMsg, wParam, lParam);
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
