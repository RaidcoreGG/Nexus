///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CrashHandler.cpp
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CrashHandler.h"

#include <dbghelp.h>
#include <errhandlingapi.h>

namespace Raidcore::Nexus::Platform
{
	static CCrashHandler* s_CrashHandler{};

	///----------------------------------------------------------------------------------------------------
	/// WriteToFile:
	/// 	Helper function to log to the specified file.
	///----------------------------------------------------------------------------------------------------
	void WriteToFile(HANDLE aFileHandle, const char* aFmt, ...)
	{
		char buffer[0x1000]{};
		char* p = &buffer[0];

		va_list args;
		va_start(args, aFmt);
		vsnprintf(p, sizeof(buffer), aFmt, args);
		::WriteFile(aFileHandle, p, static_cast<DWORD>(strlen(p)), nullptr, nullptr);
		va_end(args);
	}

	CCrashHandler::CCrashHandler(std::filesystem::path aCrashLogPath, std::filesystem::path aCrashStackPath)
	{
		::GetSystemInfo(&this->SystemInfo);

		if (this->SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		{
			// x64
			this->MachineType = IMAGE_FILE_MACHINE_AMD64;
		}
		else if (this->SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
		{
			// x86
			this->MachineType = IMAGE_FILE_MACHINE_I386;
		}
		else
		{
			throw "Unexpected CPU architecture.";
		}

		this->VEH = ::AddVectoredExceptionHandler(1, CCrashHandler::OnVectoredException);

		memset(this->LogPath, 0, MAX_PATH);
		strcpy_s(this->LogPath, MAX_PATH, aCrashLogPath.string().c_str());

		memset(this->StackPath, 0, MAX_PATH);
		strcpy_s(this->StackPath, MAX_PATH, aCrashStackPath.string().c_str());

		this->CallstackSize = 0;
		memset(this->Callstack, 0, sizeof(this->Callstack));

		if (s_CrashHandler)
		{
			throw "CCrashHandler already registered.";
		}

		s_CrashHandler = this;
	}

	CCrashHandler::~CCrashHandler()
	{
		if (this->VEH)
		{
			::RemoveVectoredExceptionHandler(this->VEH);
		}
	}

	/*static*/ LONG WINAPI CCrashHandler::OnVectoredException(EXCEPTION_POINTERS* aExcPointers)
	{
		switch (aExcPointers->ExceptionRecord->ExceptionCode)
		{
			case EXCEPTION_ACCESS_VIOLATION:
			case EXCEPTION_ILLEGAL_INSTRUCTION:
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
			case EXCEPTION_STACK_OVERFLOW:
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			case EXCEPTION_IN_PAGE_ERROR:
			case EXCEPTION_GUARD_PAGE:
			case EXCEPTION_PRIV_INSTRUCTION:
			//case 0xE06D7363: // C++ exception
			{
				break;
			}
			default:
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}
		}

		HANDLE hProcess = ::GetCurrentProcess();

		/* If we cannot initalize, we cleanup and then initialize again. Hooks 'n stuff.. */
		if (!::SymInitialize(hProcess, NULL, TRUE))
		{
			::SymCleanup(hProcess);
			::SymInitialize(hProcess, NULL, TRUE);
		}

		/* Reset the callstack. */
		s_CrashHandler->CallstackSize = 0;
		memset(s_CrashHandler->Callstack, 0, sizeof(s_CrashHandler->Callstack));

		/* Initialize the stackframe. */
		STACKFRAME64 sf{};

#if defined(_WIN64)
		sf.AddrPC.Offset = aExcPointers->ContextRecord->Rip;
		sf.AddrStack.Offset = aExcPointers->ContextRecord->Rsp;
		sf.AddrFrame.Offset = aExcPointers->ContextRecord->Rbp;
#elif defined(WIN32)
		sf.AddrPC.Offset = aExcPointers->ContextRecord->Eip;
		sf.AddrStack.Offset = aExcPointers->ContextRecord->Esp;
		sf.AddrFrame.Offset = aExcPointers->ContextRecord->Ebp;
#endif
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrStack.Mode = AddrModeFlat;
		sf.AddrFrame.Mode = AddrModeFlat;

		/* Walk through the stack frames. */
		HANDLE hThread = ::GetCurrentThread();
		while (::StackWalk64(s_CrashHandler->MachineType, hProcess, hThread, &sf, aExcPointers->ContextRecord, 0, ::SymFunctionTableAccess64, ::SymGetModuleBase64, 0))
		{
			if (sf.AddrFrame.Offset == 0)
			{
				break;
			}

			/* Resolve function name. */
			SYMBOL_INFO_PACKAGE symPack{}; // This holds SYMBOL_INFO + an additional MAX_SYN_NAME + 1.
			PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(&symPack);

			pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			pSymbol->MaxNameLen = MAX_SYM_NAME;

			IMAGEHLP_MODULE64 moduleInfo{};
			moduleInfo.SizeOfStruct = sizeof(moduleInfo);

			StackEntry_t& entry = s_CrashHandler->Callstack[s_CrashHandler->CallstackSize];

			if (::SymGetModuleInfo64(hProcess, sf.AddrPC.Offset, &moduleInfo))
			{
				entry.ModuleOffset = sf.AddrPC.Offset - moduleInfo.BaseOfImage;
				strcpy_s(entry.ModuleName, sizeof(entry.ModuleName), moduleInfo.LoadedImageName);
			}

			DWORD64 dwSymDisplacement = 0;
			if (::SymFromAddr(hProcess, sf.AddrPC.Offset, &dwSymDisplacement, pSymbol))
			{
				strcpy_s(entry.FunctionName, sizeof(entry.FunctionName), pSymbol->Name);
			}
			else
			{
				snprintf(&entry.FunctionName[0], sizeof(entry.FunctionName), "0x%llX", entry.ModuleOffset);
			}

			/* Get line and module name. */
			IMAGEHLP_LINE64 lineInfo{};
			DWORD dwLineDisplacement = 0;

			if (::SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo))
			{
				strcpy_s(entry.FileName, sizeof(entry.FileName), lineInfo.FileName);
				entry.LineNumber = lineInfo.LineNumber;
			}
			else
			{
				strcpy_s(entry.FileName, sizeof(entry.FileName), moduleInfo.ImageName);
				entry.LineNumber = -1;
			}

			/* Increment size/index. */
			s_CrashHandler->CallstackSize++;
		}

		/* Cleanup. */
		::SymCleanup(hProcess);

		/* Write log file, for in depth analysis. */
		HANDLE hLogFile = ::CreateFileA(
			s_CrashHandler->LogPath,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);

		if (hLogFile == INVALID_HANDLE_VALUE)
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}

		WriteToFile(hLogFile, "========================\n");
		WriteToFile(hLogFile, "Exception: 0x%X\n", aExcPointers->ExceptionRecord->ExceptionCode);
		WriteToFile(hLogFile, "========================\n");
		WriteToFile(hLogFile, "Stack Trace:\n");
		WriteToFile(hLogFile, "========================\n");

		size_t frameIndex = 0;
		for (size_t i = 0; i < s_CrashHandler->CallstackSize; i++)
		{
			StackEntry_t& entry = s_CrashHandler->Callstack[i];

			WriteToFile(hLogFile, "%s@%s", entry.FileName, entry.FunctionName);

			if (entry.LineNumber != -1)
			{
				WriteToFile(hLogFile, " on line %u", entry.LineNumber);
			}

			WriteToFile(hLogFile, "\n");
		}

		WriteToFile(hLogFile, "========================\n");

		::CloseHandle(hLogFile);

		/* Write stack log, to let loader analyze what to disable next launch. */
		HANDLE hStackFile = ::CreateFileA(
			s_CrashHandler->StackPath,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);

		if (hStackFile == INVALID_HANDLE_VALUE)
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}

		for (size_t i = 0; i < s_CrashHandler->CallstackSize; i++)
		{
			StackEntry_t& entry = s_CrashHandler->Callstack[i];
			WriteToFile(hStackFile, "%s\n", entry.FileName);
		}

		::CloseHandle(hStackFile);

		//::ShellExecuteA(nullptr, "open", s_CrashHandler->LogPath, nullptr, nullptr, SW_SHOWNORMAL);

		return EXCEPTION_EXECUTE_HANDLER;
	}
}
