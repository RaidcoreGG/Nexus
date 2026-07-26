///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PlContext.cpp
/// Description  :  Platform context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PlContext.h"

#include <filesystem>
#include <memory>
#include <windows.h>

#include "CrashHandler/CrashHandler.h"
#include "Index/IdxEnum.h"
#include "Index/Index.h"

namespace Raidcore::Nexus::Platform
{
	Context::Context()
	{
		EnumWindows([](HWND aHandle, LPARAM aOutHandle) -> BOOL
		{
			DWORD pid = 0;
			GetWindowThreadProcessId(aHandle, &pid);

			if (GetCurrentProcessId() != pid)
			{
				return TRUE;
			}

			if (GetWindow(aHandle, GW_OWNER) != nullptr)
			{
				return TRUE;
			}

			if (!IsWindowVisible(aHandle))
			{
				return TRUE;
			}

			*reinterpret_cast<HWND*>(aOutHandle) = aHandle;
			return FALSE;
		}, reinterpret_cast<LPARAM>(&this->_WindowHandle));

		this->_CrashHandler = std::make_unique<Platform::CrashHandler>(
			Index(EPath::CrashLog),
			Index(EPath::CrashStack)
		);
		this->_RawInputApi = std::make_unique<RawInputApi>();
	}

	void Context::Shutdown()
	{
		this->_CrashHandler.reset();
		this->_RawInputApi.reset();
	}

	HWND Context::Window()
	{
		return this->_WindowHandle;
	}

	Platform::CrashHandler& Context::CrashHandler()
	{
		return *this->_CrashHandler;
	}

	Platform::RawInputApi& Context::RawInput()
	{
		return *this->_RawInputApi;
	}
}
