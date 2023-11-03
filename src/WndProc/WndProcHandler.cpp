#include "WndProcHandler.h"

namespace WndProc
{
	std::mutex						Mutex;
	std::vector<WNDPROC_CALLBACK>	Registry;

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// don't pass to game if addon wndproc
		WndProc::Mutex.lock();
		{
			for (WNDPROC_CALLBACK wndprocCb : Registry)
			{
				if (wndprocCb(hWnd, uMsg, wParam, lParam) == 0)
				{
					WndProc::Mutex.unlock();
					return 0;
				}
			}
		}
		WndProc::Mutex.unlock();

		return 1;
	}

	void Register(WNDPROC_CALLBACK aWndProcCallback)
	{
		WndProc::Mutex.lock();
		{
			Registry.push_back(aWndProcCallback);
		}
		WndProc::Mutex.unlock();
	}

	void Unregister(WNDPROC_CALLBACK aWndProcCallback)
	{
		WndProc::Mutex.lock();
		{
			Registry.erase(std::remove(Registry.begin(), Registry.end(), aWndProcCallback), Registry.end());
		}
		WndProc::Mutex.unlock();
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		WndProc::Mutex.lock();
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
		WndProc::Mutex.unlock();

		return refCounter;
	}
}