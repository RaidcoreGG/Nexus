///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WndProcHandler.cpp
/// Description  :  Proxy for WndProc callbacks/hooks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "WndProcHandler.h"

#include "Hooks.h"

namespace WndProc
{
	std::mutex						Mutex;
	std::vector<WNDPROC_CALLBACK>	Registry;

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// don't pass to game if addon wndproc
		const std::lock_guard<std::mutex> lock(Mutex);
		for (WNDPROC_CALLBACK wndprocCb : Registry)
		{
			if (wndprocCb(hWnd, uMsg, wParam, lParam) == 0)
			{
				return 0;
			}
		}

		return 1;
	}

	LRESULT SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return CallWindowProcA(Hooks::GW2::WndProc, hWnd, uMsg, wParam, lParam);
	}

	void Register(WNDPROC_CALLBACK aWndProcCallback)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		Registry.push_back(aWndProcCallback);
	}

	void Deregister(WNDPROC_CALLBACK aWndProcCallback)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		Registry.erase(std::remove(Registry.begin(), Registry.end(), aWndProcCallback), Registry.end());
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		const std::lock_guard<std::mutex> lock(Mutex);
		for (WNDPROC_CALLBACK wndprocCb : Registry)
		{
			if (wndprocCb >= aStartAddress && wndprocCb <= aEndAddress)
			{
				Registry.erase(std::remove(Registry.begin(), Registry.end(), wndprocCb), Registry.end());
				refCounter++;
			}
		}

		return refCounter;
	}
}
