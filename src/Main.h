#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

namespace Main
{
	void Initialize();
	void Shutdown(unsigned int aReason);

	void SelfUpdate();

	void UnpackLocales();
}

#endif
