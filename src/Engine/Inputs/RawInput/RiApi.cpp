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
	{
		const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

		std::shared_ptr<const std::vector<WNDPROC_CALLBACK>> registry = this->Registry;

		/* Addons first, before any native subscriber. */
		for (WNDPROC_CALLBACK wndprocCb : *registry)
		{
			if (wndprocCb(hWnd, uMsg, wParam, lParam) == 0)
			{
				return 0;
			}
		}
	}

	return this->NotifyWndProc(hWnd, uMsg, wParam, lParam);
}

void CRawInputApi::Register(WNDPROC_CALLBACK aWndProcCallback)
{
	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

	std::shared_ptr<std::vector<WNDPROC_CALLBACK>> registry = std::make_shared<std::vector<WNDPROC_CALLBACK>>(*this->Registry);
	registry->push_back(aWndProcCallback);
	this->Registry = registry;
}

void CRawInputApi::Deregister(WNDPROC_CALLBACK aWndProcCallback)
{
	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

	std::shared_ptr<std::vector<WNDPROC_CALLBACK>> registry = std::make_shared<std::vector<WNDPROC_CALLBACK>>(*this->Registry);
	std::erase(*registry, aWndProcCallback);
	this->Registry = registry;
}

uint32_t CRawInputApi::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

	std::shared_ptr<std::vector<WNDPROC_CALLBACK>> registry = std::make_shared<std::vector<WNDPROC_CALLBACK>>(*this->Registry);

	size_t removed = std::erase_if(*registry, [aStartAddress, aEndAddress](WNDPROC_CALLBACK aCallback)
	{
		return aCallback >= aStartAddress && aCallback <= aEndAddress;
	});

	this->Registry = registry;

	return static_cast<uint32_t>(removed);
}
