///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DLL.cpp
/// Description  :  Contains functions for DLLs.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "DLL.h"

namespace DLL
{
	bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName)
	{
		FARPROC* fp = (FARPROC*)aFunction;
		*fp = aModule ? GetProcAddress(aModule, aName) : 0;
		return (*fp != 0);
	}
}
