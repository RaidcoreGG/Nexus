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
		static uint32_t s_Dummy = 0;

		HMODULE hmodule = nullptr;
		GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			(LPCSTR)&s_Dummy,
			&hmodule
		);

		this->_Module = hmodule;

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

	HMODULE Context::Module()
	{
		return this->_Module;
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
