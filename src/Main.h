#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>

#include "Proxy.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

namespace Main
{
	void Initialize(EEntryMethod aEntryMethod);

	void Shutdown(unsigned int aReason);
}

#endif
