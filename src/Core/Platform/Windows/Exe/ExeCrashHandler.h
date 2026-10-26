///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExeCrashHandler.h
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <filesystem>

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

	///----------------------------------------------------------------------------------------------------
	/// Reattach:
	/// 	Re-enables the SetUnhandledExceptionFilter hook after all hooks were disabled.
	///----------------------------------------------------------------------------------------------------
	void Reattach();

	private:
	char                         LogPath[MAX_PATH];
	char                         StackPath[MAX_PATH];
	LPTOP_LEVEL_EXCEPTION_FILTER PrevFilter;        /* Runs after ours: set before us, or the latest set after us. */
	LPVOID                       SetFilterTarget;
	LPVOID                       SetFilterOriginal;
	SYSTEM_INFO                  SystemInfo;
	DWORD                        MachineType;
	StackEntry_t                 Callstack[64];
	size_t                       CallstackSize;

	///----------------------------------------------------------------------------------------------------
	/// OnUnhandledException:
	/// 	Logs the unhandled exception, then passes it to the previous filter.
	///----------------------------------------------------------------------------------------------------
	static LONG WINAPI OnUnhandledException(EXCEPTION_POINTERS* aExcPointers);

	///----------------------------------------------------------------------------------------------------
	/// OnSetUnhandledExceptionFilter:
	/// 	Detour that keeps our filter installed and stores the new one to run after ours.
	///----------------------------------------------------------------------------------------------------
	static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI OnSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER aFilter);
};
