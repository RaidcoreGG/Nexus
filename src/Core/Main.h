#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <dxgi.h>

#include "Proxy/PxyEnum.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

namespace Main
{
	void Initialize(EProxyFunction aEntryFunction);

	void Shutdown(unsigned int aReason);
}

#endif
