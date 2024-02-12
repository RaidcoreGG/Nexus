#ifndef WNDPROCHANDLER_H
#define WNDPROCHANDLER_H

#include <vector>
#include <mutex>

#include "FuncDefs.h"

namespace WndProc
{
	extern std::mutex						Mutex;
	extern std::vector<WNDPROC_CALLBACK>	Registry;

	/* Returns 0 if message was processed. */
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/* Registers the provided WndProcCallback. */
	void Register(WNDPROC_CALLBACK aWndProcCallback);
	/* Deregisters the provided WndProcCallback. */
	void Deregister(WNDPROC_CALLBACK aWndProcCallback);

	/* Removes all WndProc Callbacks that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif