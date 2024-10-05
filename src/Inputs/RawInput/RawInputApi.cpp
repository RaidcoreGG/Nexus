///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WndProcHandler.cpp
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RawInputApi.h"

#include "Hooks.h"
#include "Renderer.h"
#include "Shared.h"

namespace RawInput
{
	LRESULT ADDONAPI_SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return RawInputApi->SendWndProcToGame(hWnd, uMsg, wParam, lParam);
	}

	void ADDONAPI_Register(WNDPROC_CALLBACK aWndProcCallback)
	{
		RawInputApi->Register(aWndProcCallback);
	}

	void ADDONAPI_Deregister(WNDPROC_CALLBACK aWndProcCallback)
	{
		RawInputApi->Deregister(aWndProcCallback);
	}
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

LRESULT CRawInputApi::SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return PostMessage(Renderer::WindowHandle, uMsg, wParam, lParam);
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
