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
#include <fstream>

struct FunctionCall
{
	std::string FunctionName;
	std::string FileName;
	uint32_t    LineNumber;
	uint64_t    ModuleOffset;
};

class CCrashHandler
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CCrashHandler(std::filesystem::path aLogPath);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CCrashHandler();

	private:
	std::filesystem::path        LogPath;
	std::ofstream                LogStream;
	PVOID                        VEH;
	LPTOP_LEVEL_EXCEPTION_FILTER UEF;
	SYSTEM_INFO                  SystemInfo;
	DWORD                        MachineType;

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
