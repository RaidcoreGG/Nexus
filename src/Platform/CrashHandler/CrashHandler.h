///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CrashHandler.h
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <windows.h>

///----------------------------------------------------------------------------------------------------
/// StackEntry_t Struct
///----------------------------------------------------------------------------------------------------
struct StackEntry_t
{
	char     FileName[MAX_PATH];
	char     FunctionName[MAX_PATH];
	uint32_t LineNumber;
	char     ModuleName[MAX_PATH];
	uint64_t ModuleOffset;
};

///----------------------------------------------------------------------------------------------------
/// CCrashHandler Class
///----------------------------------------------------------------------------------------------------
class CCrashHandler
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CCrashHandler(std::filesystem::path aCrashLogPath, std::filesystem::path aCrashStackPath);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CCrashHandler();

	private:
	char                         LogPath[MAX_PATH];
	char                         StackPath[MAX_PATH];
	PVOID                        VEH;
	SYSTEM_INFO                  SystemInfo;
	DWORD                        MachineType;
	StackEntry_t                 Callstack[64];
	size_t                       CallstackSize;

	///----------------------------------------------------------------------------------------------------
	/// OnVectoredException:
	/// 	Logs the unhandled exception.
	///----------------------------------------------------------------------------------------------------
	static LONG WINAPI OnVectoredException(EXCEPTION_POINTERS* aExcPointers);
};
