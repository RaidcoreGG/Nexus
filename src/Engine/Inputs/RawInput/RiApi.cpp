///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RiApi.cpp
/// Description  :  API for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RiApi.h"

CRawInputApi::CRawInputApi() : IRefCleaner("RawInputApi")
{
}

CRawInputApi::~CRawInputApi()
{
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

uint32_t CRawInputApi::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	uint32_t refCounter = 0;

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
