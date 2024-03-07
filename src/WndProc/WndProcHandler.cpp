#include "WndProcHandler.h"

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
				WndProc::Mutex.unlock();
				return 0;
			}
		}

		return 1;
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