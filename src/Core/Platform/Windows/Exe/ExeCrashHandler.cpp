///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExeCrashHandler.cpp
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ExeCrashHandler.h"

#include <dbghelp.h>
#include <errhandlingapi.h>

#include "minhook/mh_hook.h"

typedef LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI* SETUNHANDLEDEXCEPTIONFILTER)(LPTOP_LEVEL_EXCEPTION_FILTER aFilter);

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
	DWORD written = 0;
	::WriteFile(aFileHandle, p, static_cast<DWORD>(strlen(p)), &written, nullptr);
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

	/* Filters set later, e.g. by ArcDPS, would replace ours. The hook keeps ours first and runs theirs after it. */
	HMODULE kernelBase = ::GetModuleHandleW(L"kernelbase.dll");
	this->SetFilterTarget   = kernelBase ? ::GetProcAddress(kernelBase, "SetUnhandledExceptionFilter") : nullptr;
	this->SetFilterOriginal = nullptr;

	if (!this->SetFilterTarget ||
		MH_CreateHook(this->SetFilterTarget, (LPVOID)&CCrashHandler::OnSetUnhandledExceptionFilter, &this->SetFilterOriginal) != EMHStatus::MH_OK ||
		MH_EnableHook(this->SetFilterTarget) != EMHStatus::MH_OK)
	{
		this->SetFilterTarget   = nullptr;
		this->SetFilterOriginal = nullptr;
	}

	SETUNHANDLEDEXCEPTIONFILTER setFilter = this->SetFilterOriginal
		? (SETUNHANDLEDEXCEPTIONFILTER)this->SetFilterOriginal
		: ::SetUnhandledExceptionFilter;

	/* Only reached by exceptions nobody handled. The previous filter, e.g. the game's, runs after ours. */
	this->PrevFilter = setFilter(CCrashHandler::OnUnhandledException);
}

CCrashHandler::~CCrashHandler()
{
	SETUNHANDLEDEXCEPTIONFILTER setFilter = this->SetFilterOriginal
		? (SETUNHANDLEDEXCEPTIONFILTER)this->SetFilterOriginal
		: ::SetUnhandledExceptionFilter;

	setFilter(this->PrevFilter);
}

void CCrashHandler::Reattach()
{
	if (this->SetFilterTarget)
	{
		MH_EnableHook(this->SetFilterTarget);
	}
}

/*static*/ LPTOP_LEVEL_EXCEPTION_FILTER WINAPI CCrashHandler::OnSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER aFilter)
{
	if (aFilter == CCrashHandler::OnUnhandledException)
	{
		return s_CrashHandler->PrevFilter;
	}

	/* Returns the one stored before, never ours: a caller that chains to it would call back into us. */
	return (LPTOP_LEVEL_EXCEPTION_FILTER)::InterlockedExchangePointer((PVOID volatile*)&s_CrashHandler->PrevFilter, (PVOID)aFilter);
}

/*static*/ LONG WINAPI CCrashHandler::OnUnhandledException(EXCEPTION_POINTERS* aExcPointers)
{
	/* Write log file, for in depth analysis. Opened first, so the exception is logged even if symbol lookup fails. */
	HANDLE hLogFile = ::CreateFileA(
		s_CrashHandler->LogPath,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		WriteToFile(hLogFile, "========================\n");
		WriteToFile(hLogFile, "Exception: 0x%X at 0x%p\n", aExcPointers->ExceptionRecord->ExceptionCode, aExcPointers->ExceptionRecord->ExceptionAddress);
		WriteToFile(hLogFile, "========================\n");
	}

	/* Own handle, so this dbghelp session does not collide with one the game or an addon opened. */
	HANDLE hProcess = nullptr;
	::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentProcess(), ::GetCurrentProcess(), &hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);
	::SymInitialize(hProcess, NULL, TRUE);

	/* Reset the callstack. */
	s_CrashHandler->CallstackSize = 0;
	memset(s_CrashHandler->Callstack, 0, sizeof(s_CrashHandler->Callstack));

	/* StackWalk64 modifies the context, the original is still needed by the next filter. */
	CONTEXT context = *aExcPointers->ContextRecord;

	/* Initialize the stackframe. */
	STACKFRAME64 sf{};

#if defined(_WIN64)
	sf.AddrPC.Offset = context.Rip;
	sf.AddrStack.Offset = context.Rsp;
	sf.AddrFrame.Offset = context.Rbp;
#elif defined(WIN32)
	sf.AddrPC.Offset = context.Eip;
	sf.AddrStack.Offset = context.Esp;
	sf.AddrFrame.Offset = context.Ebp;
#endif
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;

	/* Walk through the stack frames. */
	HANDLE hThread = ::GetCurrentThread();
	while (s_CrashHandler->CallstackSize < _countof(s_CrashHandler->Callstack) &&
		::StackWalk64(s_CrashHandler->MachineType, hProcess, hThread, &sf, &context, 0, ::SymFunctionTableAccess64, ::SymGetModuleBase64, 0))
	{
		/* The frame pointer is often 0 on x64, the end of the stack is a 0 program counter. */
		if (sf.AddrPC.Offset == 0)
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
		lineInfo.SizeOfStruct = sizeof(lineInfo);
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
	::CloseHandle(hProcess);

	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		WriteToFile(hLogFile, "Stack Trace:\n");
		WriteToFile(hLogFile, "========================\n");

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
	}

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

	if (hStackFile != INVALID_HANDLE_VALUE)
	{
		for (size_t i = 0; i < s_CrashHandler->CallstackSize; i++)
		{
			StackEntry_t& entry = s_CrashHandler->Callstack[i];

			/* The module, not the source file, identifies the addon. */
			if (entry.ModuleName[0])
			{
				WriteToFile(hStackFile, "%s\n", entry.ModuleName);
			}
		}

		::CloseHandle(hStackFile);
	}

	//::ShellExecuteA(nullptr, "open", s_CrashHandler->LogPath, nullptr, nullptr, SW_SHOWNORMAL);

	LPTOP_LEVEL_EXCEPTION_FILTER next = s_CrashHandler->PrevFilter;

	/* Skip the filter of an addon that was unloaded without restoring the previous one. */
	HMODULE nextModule = nullptr;
	if (next && ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)next, &nextModule))
	{
		return next(aExcPointers);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
