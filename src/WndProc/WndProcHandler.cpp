#include "WndProcHandler.h"

namespace WndProc
{
	std::mutex						Mutex;
	std::vector<WNDPROC_CALLBACK>	Registry;

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// don't pass to game if addon wndproc
		Mutex.lock();
		{
			for (WNDPROC_CALLBACK wndprocCb : Registry)
			{
				if (wndprocCb(hWnd, uMsg, wParam, lParam) == 0)
				{
					Mutex.unlock();
					return 0;
				}
			}
		}
		Mutex.unlock();

		return 1;
	}

	void Register(WNDPROC_CALLBACK aWndProcCallback)
	{
		Mutex.lock();
		{
			Registry.push_back(aWndProcCallback);
		}
		Mutex.unlock();
	}

	void Unregister(WNDPROC_CALLBACK aWndProcCallback)
	{
		Mutex.lock();
		{
			Registry.erase(std::remove(Registry.begin(), Registry.end(), aWndProcCallback), Registry.end());
		}
		Mutex.unlock();
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Mutex.lock();
		{
			for (WNDPROC_CALLBACK wndprocCb : Registry)
			{
				if (wndprocCb >= aStartAddress && wndprocCb <= aEndAddress)
				{
					Registry.erase(std::remove(Registry.begin(), Registry.end(), wndprocCb), Registry.end());
					refCounter++;
				}
			}
		}
		Mutex.unlock();

		return refCounter;
	}
}