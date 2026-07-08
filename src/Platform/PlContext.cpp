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

namespace Raidcore::Nexus::Platform
{
	Context::Context(std::filesystem::path aCrashlog, std::filesystem::path aCrashstack)
		: CrashLogPath(std::move(aCrashlog))
		, CrashStackPath(std::move(aCrashstack))
	{
		this->_CrashHandler = std::make_unique<CCrashHandler>(
			this->CrashLogPath,
			this->CrashStackPath
		);

		this->_RawInputApi = std::make_unique<CRawInputApi>();
	}

	void Context::Shutdown()
	{
		this->_CrashHandler.reset();
		this->_RawInputApi.reset();
	}

	HMODULE Context::Module()
	{
		if (!this->_Module)
		{
			static uint32_t s_Dummy = 0;

			HMODULE hmodule = nullptr;
			GetModuleHandleExA(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCSTR)&s_Dummy,
				&hmodule
			);

			this->_Module = hmodule;
		}

		return this->_Module;
	}

	CCrashHandler& Context::CrashHandler()
	{
		return *this->_CrashHandler;
	}

	CRawInputApi& Context::RawInput()
	{
		return *this->_RawInputApi;
	}
}
