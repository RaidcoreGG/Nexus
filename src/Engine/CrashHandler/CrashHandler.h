///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CrashHandler.h
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <errhandlingapi.h>
#include <filesystem>

struct StackEntry_t
{
	char     FunctionName[MAX_PATH];
	char     FileName[MAX_PATH];
	uint32_t LineNumber;
	uint64_t ModuleOffset;
};

class CCrashHandler
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CCrashHandler(std::filesystem::path aCrashLogPath);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CCrashHandler();

	private:
	char                         LogPath[MAX_PATH];
	PVOID                        VEH;
	LPTOP_LEVEL_EXCEPTION_FILTER UEF;
	SYSTEM_INFO                  SystemInfo;
	DWORD                        MachineType;
	StackEntry_t                 Callstack[64];
	size_t                       CallstackSize;

	///----------------------------------------------------------------------------------------------------
	/// OnVectoredException:
	/// 	Logs the unhandled exception.
	///----------------------------------------------------------------------------------------------------
	static LONG WINAPI OnVectoredException(EXCEPTION_POINTERS* aExcPointers);

	///----------------------------------------------------------------------------------------------------
	/// OnUnhandledException:
	/// 	Logs the unhandled exception.
	///----------------------------------------------------------------------------------------------------
	static LONG WINAPI OnUnhandledException(EXCEPTION_POINTERS* aExcPointers);
};
