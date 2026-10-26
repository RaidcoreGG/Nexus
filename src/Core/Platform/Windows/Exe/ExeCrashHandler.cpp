///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExeCrashHandler.cpp
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ExeCrashHandler.h"

#include <cerrno>
#include <cstdio>
#include <dbghelp.h>
#include <errhandlingapi.h>
#include <intrin.h>
#include <psapi.h>

#include "minhook/mh_hook.h"

typedef LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI* SETUNHANDLEDEXCEPTIONFILTER)(LPTOP_LEVEL_EXCEPTION_FILTER aFilter);
typedef LONG (WINAPI* RTLGETVERSION)(PRTL_OSVERSIONINFOW aVersionInfo);

static CCrashHandler* s_CrashHandler{};

/* Static instead of on the stack: the filter may run on a thread that just overflowed its stack. */
static char    s_UserProfile[MAX_PATH]{};
static HMODULE s_Modules[1024]{};

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

///----------------------------------------------------------------------------------------------------
/// ShortenPath:
/// 	Replaces the user profile directory with %USERPROFILE%, so logs shared by users carry no names.
///----------------------------------------------------------------------------------------------------
static const char* ShortenPath(const char* aPath, char* aBuffer, size_t aBufferSize)
{
	size_t profileLength = strlen(s_UserProfile);

	if (profileLength > 0 && _strnicmp(aPath, s_UserProfile, profileLength) == 0)
	{
		snprintf(aBuffer, aBufferSize, "%%USERPROFILE%%%s", aPath + profileLength);
		return aBuffer;
	}

	return aPath;
}

///----------------------------------------------------------------------------------------------------
/// GetModuleOf:
/// 	Writes the shortened path of the module containing aAddress and returns its base, or 0.
///----------------------------------------------------------------------------------------------------
static uintptr_t GetModuleOf(const void* aAddress, char* aBuffer, size_t aBufferSize)
{
	HMODULE module = nullptr;

	if (!::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)aAddress, &module))
	{
		strcpy_s(aBuffer, aBufferSize, "?");
		return 0;
	}

	char path[MAX_PATH]{};
	char shortPath[MAX_PATH]{};
	::GetModuleFileNameA(module, path, MAX_PATH);
	strcpy_s(aBuffer, aBufferSize, ShortenPath(path, shortPath, sizeof(shortPath)));

	return (uintptr_t)module;
}

///----------------------------------------------------------------------------------------------------
/// WriteInfo:
/// 	Exception code, address, parameters and the system's description of the code.
///----------------------------------------------------------------------------------------------------
static void WriteInfo(HANDLE aFile, const EXCEPTION_RECORD* aRecord)
{
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----Info--------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "Code:    0x%08X\n", aRecord->ExceptionCode);
	WriteToFile(aFile, "Address: 0x%p\n", aRecord->ExceptionAddress);
	WriteToFile(aFile, "Flags:   0x%X\n", aRecord->ExceptionFlags);

	for (DWORD i = 0; i < aRecord->NumberParameters && i < EXCEPTION_MAXIMUM_PARAMETERS; i++)
	{
		WriteToFile(aFile, "Param%u:  0x%llX\n", i, (unsigned long long)aRecord->ExceptionInformation[i]);
	}

	if (aRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && aRecord->NumberParameters >= 2)
	{
		const char* operation = "read";
		if (aRecord->ExceptionInformation[0] == 1) { operation = "write"; }
		if (aRecord->ExceptionInformation[0] == 8) { operation = "execute"; }

		WriteToFile(aFile, "Access:  %s at 0x%llX\n", operation, (unsigned long long)aRecord->ExceptionInformation[1]);
	}

	char templ[512]{};
	if (::FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		::GetModuleHandleW(L"ntdll.dll"), aRecord->ExceptionCode, 0, templ, sizeof(templ), nullptr))
	{
		/* NT messages span several lines, e.g. "{EXCEPTION}\r\nInteger division by zero.\r\n". */
		for (char* c = templ; *c; c++)
		{
			if (*c == '\r' || *c == '\n') { *c = ' '; }
		}

		char text[512]{};

		/* The placeholders are printf-style; only fill them where the arguments are known. */
		if ((aRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || aRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) && aRecord->NumberParameters >= 2)
		{
			const char* operation = "read";
			if (aRecord->ExceptionInformation[0] == 1) { operation = "written"; }
			if (aRecord->ExceptionInformation[0] == 8) { operation = "executed"; }

			void* target = (void*)aRecord->ExceptionInformation[1];

			if (aRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				snprintf(text, sizeof(text), templ, aRecord->ExceptionAddress, target, operation);
			}
			else if (aRecord->NumberParameters >= 3)
			{
				snprintf(text, sizeof(text), templ, aRecord->ExceptionAddress, target, (unsigned)aRecord->ExceptionInformation[2]);
			}
		}

		if (!text[0])
		{
			strcpy_s(text, templ);
		}

		/* Drop the trailing space left by the final line break. */
		size_t length = strlen(text);
		while (length > 0 && text[length - 1] == ' ') { text[--length] = '\0'; }

		WriteToFile(aFile, "Text:    %s\n", text);
	}
}

///----------------------------------------------------------------------------------------------------
/// WriteSystem:
/// 	Windows version, CPU, memory and the process.
///----------------------------------------------------------------------------------------------------
static void WriteSystem(HANDLE aFile, const SYSTEM_INFO& aSystemInfo)
{
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----System------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");

	/* GetVersionEx reports what the process is manifested for, RtlGetVersion the real version. */
	RTLGETVERSION rtlGetVersion = (RTLGETVERSION)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
	RTL_OSVERSIONINFOW version{};
	version.dwOSVersionInfoSize = sizeof(version);
	if (rtlGetVersion && rtlGetVersion(&version) == 0)
	{
		/* Windows 11 still reports 10.0, only the build number tells them apart. */
		DWORD major = (version.dwMajorVersion == 10 && version.dwBuildNumber >= 22000) ? 11 : version.dwMajorVersion;
		WriteToFile(aFile, "windows: %u.%u.%u\n", major, version.dwMinorVersion, version.dwBuildNumber);
	}

	int  cpuInfo[4]{};
	char brand[0x40]{};
	__cpuid(cpuInfo, 0x80000000);
	if (static_cast<unsigned>(cpuInfo[0]) >= 0x80000004)
	{
		__cpuid(reinterpret_cast<int*>(brand),        0x80000002);
		__cpuid(reinterpret_cast<int*>(brand + 0x10), 0x80000003);
		__cpuid(reinterpret_cast<int*>(brand + 0x20), 0x80000004);
	}
	WriteToFile(aFile, "CPU:     %s (%u threads)\n", brand, aSystemInfo.dwNumberOfProcessors);

	MEMORYSTATUSEX memory{};
	memory.dwLength = sizeof(memory);
	if (::GlobalMemoryStatusEx(&memory))
	{
		WriteToFile(aFile, "RAM:     %lluMB used, %lluMB total\n", (memory.ullTotalPhys - memory.ullAvailPhys) >> 20, memory.ullTotalPhys >> 20);
	}

	char exePath[MAX_PATH]{};
	char shortPath[MAX_PATH]{};
	::GetModuleFileNameA(nullptr, exePath, MAX_PATH);
	WriteToFile(aFile, "Process: %s (%u)\n", ShortenPath(exePath, shortPath, sizeof(shortPath)), ::GetCurrentProcessId());
}

///----------------------------------------------------------------------------------------------------
/// WriteRegisters:
/// 	Register state at the time of the exception.
///----------------------------------------------------------------------------------------------------
static void WriteRegisters(HANDLE aFile, const CONTEXT& aContext, DWORD aLastError, int aErrno)
{
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----Registers---------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");

#if defined(_WIN64)
	WriteToFile(aFile, "RAX: 0x%016llX R8:  0x%016llX\n", aContext.Rax, aContext.R8);
	WriteToFile(aFile, "RBX: 0x%016llX R9:  0x%016llX\n", aContext.Rbx, aContext.R9);
	WriteToFile(aFile, "RCX: 0x%016llX R10: 0x%016llX\n", aContext.Rcx, aContext.R10);
	WriteToFile(aFile, "RDX: 0x%016llX R11: 0x%016llX\n", aContext.Rdx, aContext.R11);
	WriteToFile(aFile, "RBP: 0x%016llX R12: 0x%016llX\n", aContext.Rbp, aContext.R12);
	WriteToFile(aFile, "RSP: 0x%016llX R13: 0x%016llX\n", aContext.Rsp, aContext.R13);
	WriteToFile(aFile, "RSI: 0x%016llX R14: 0x%016llX\n", aContext.Rsi, aContext.R14);
	WriteToFile(aFile, "RDI: 0x%016llX R15: 0x%016llX\n", aContext.Rdi, aContext.R15);
	WriteToFile(aFile, "RIP: 0x%016llX\n", aContext.Rip);
	WriteToFile(aFile, "RFLAGS: 0x%08X MXCSR: 0x%08X\n", aContext.EFlags, aContext.MxCsr);
#else
	WriteToFile(aFile, "EAX: 0x%08X ESI: 0x%08X\n", aContext.Eax, aContext.Esi);
	WriteToFile(aFile, "EBX: 0x%08X EDI: 0x%08X\n", aContext.Ebx, aContext.Edi);
	WriteToFile(aFile, "ECX: 0x%08X EBP: 0x%08X\n", aContext.Ecx, aContext.Ebp);
	WriteToFile(aFile, "EDX: 0x%08X ESP: 0x%08X\n", aContext.Edx, aContext.Esp);
	WriteToFile(aFile, "EIP: 0x%08X\n", aContext.Eip);
	WriteToFile(aFile, "EFLAGS: 0x%08X\n", aContext.EFlags);
#endif
	WriteToFile(aFile, "CS: 0x%04X SS: 0x%04X DS: 0x%04X ES: 0x%04X FS: 0x%04X GS: 0x%04X\n",
		aContext.SegCs, aContext.SegSs, aContext.SegDs, aContext.SegEs, aContext.SegFs, aContext.SegGs);
	WriteToFile(aFile, "GetLastError: %u errno: %d\n", aLastError, aErrno);
	WriteToFile(aFile, "");
}

///----------------------------------------------------------------------------------------------------
/// WriteMemory:
/// 	Hex dump around aCenter, 16 bytes per line. Unreadable memory is shown as ??.
///----------------------------------------------------------------------------------------------------
static void WriteMemory(HANDLE aFile, uintptr_t aCenter, const char* aCenterLabel, int aBefore, int aAfter)
{
	for (int offset = -aBefore; offset < aAfter; offset += 16)
	{
		uintptr_t address = aCenter + offset;
		uint8_t   bytes[16]{};
		SIZE_T    read = 0;

		/* ReadProcessMemory fails on unmapped memory instead of faulting again. */
		bool isReadable = ::ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)address, bytes, sizeof(bytes), &read) && read == sizeof(bytes);

		char label[16]{};
		if (offset == 0)
		{
			strcpy_s(label, aCenterLabel);
		}
		else
		{
			snprintf(label, sizeof(label), "%c%X", offset < 0 ? '-' : '+', offset < 0 ? -offset : offset);
		}

		char line[128]{};
		int  length = snprintf(line, sizeof(line), "0x%p %4s:", (void*)address, label);

		for (int i = 0; i < 16; i++)
		{
			length += snprintf(line + length, sizeof(line) - length, "%s%02X", (i % 4 == 0) ? " " : "", bytes[i]);

			if (!isReadable)
			{
				line[length - 2] = '?';
				line[length - 1] = '?';
			}
		}

		WriteToFile(aFile, "%s\n", line);
	}
}

///----------------------------------------------------------------------------------------------------
/// WriteModules:
/// 	Every loaded module with its address range, to attribute addresses to addons.
///----------------------------------------------------------------------------------------------------
static void WriteModules(HANDLE aFile)
{
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----Modules-----------------------------------------------------------------------------------------\n");
	WriteToFile(aFile, "----------------------------------------------------------------------------------------------------\n");

	DWORD needed = 0;
	if (!::EnumProcessModules(::GetCurrentProcess(), s_Modules, sizeof(s_Modules), &needed))
	{
		return;
	}

	size_t count = needed / sizeof(HMODULE);
	if (count > _countof(s_Modules)) { count = _countof(s_Modules); }

	for (size_t i = 0; i < count; i++)
	{
		MODULEINFO info{};
		::GetModuleInformation(::GetCurrentProcess(), s_Modules[i], &info, sizeof(info));

		char path[MAX_PATH]{};
		char shortPath[MAX_PATH]{};
		::GetModuleFileNameA(s_Modules[i], path, MAX_PATH);

		WriteToFile(aFile, "0x%p 0x%p %s\n", info.lpBaseOfDll, (uint8_t*)info.lpBaseOfDll + info.SizeOfImage, ShortenPath(path, shortPath, sizeof(shortPath)));
	}
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

	::GetEnvironmentVariableA("USERPROFILE", s_UserProfile, MAX_PATH);

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
	/* Read before any call below overwrites them. */
	DWORD lastError = ::GetLastError();
	int   lastErrno = errno;

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

	/* Raw state first, it needs no symbols and is written even if the stack walk fails. */
	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		const EXCEPTION_RECORD* record  = aExcPointers->ExceptionRecord;
		const CONTEXT*          context = aExcPointers->ContextRecord;

		SYSTEMTIME time{};
		::GetLocalTime(&time);

		char      moduleName[MAX_PATH]{};
		uintptr_t moduleBase = GetModuleOf(record->ExceptionAddress, moduleName, sizeof(moduleName));

		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "%04u-%02u-%02u %02u:%02u:%02u.%03u: unhandled exception 0x%08X at \"%s+0x%llX\" thread %u\n",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			record->ExceptionCode, moduleName, (unsigned long long)((uintptr_t)record->ExceptionAddress - moduleBase), ::GetCurrentThreadId());

		WriteInfo(hLogFile, record);
		WriteSystem(hLogFile, s_CrashHandler->SystemInfo);
		WriteRegisters(hLogFile, *context, lastError, lastErrno);

#if defined(_WIN64)
		uintptr_t instructionPointer = context->Rip;
		uintptr_t stackPointer       = context->Rsp;
#else
		uintptr_t instructionPointer = context->Eip;
		uintptr_t stackPointer       = context->Esp;
#endif

		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "----Code--------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteMemory(hLogFile, instructionPointer, "IP", 0x40, 0x50);
		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "----Stack-------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteMemory(hLogFile, stackPointer, "SP", 0x60, 0x200);
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
		entry.Address = sf.AddrPC.Offset;
		entry.Frame   = sf.AddrFrame.Offset;

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
		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "----Trace-------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");
		WriteToFile(hLogFile, "Address------------Frame--------------Module--------------------------------------------------------\n");

		for (size_t i = 0; i < s_CrashHandler->CallstackSize; i++)
		{
			StackEntry_t& entry = s_CrashHandler->Callstack[i];

			const char* moduleFile = strrchr(entry.ModuleName, '\\');
			moduleFile = moduleFile ? moduleFile + 1 : (entry.ModuleName[0] ? entry.ModuleName : "?");

			WriteToFile(hLogFile, "0x%016llX 0x%016llX %s+0x%llX %s", entry.Address, entry.Frame, moduleFile, entry.ModuleOffset, entry.FunctionName);

			if (entry.LineNumber != -1)
			{
				char shortPath[MAX_PATH]{};
				WriteToFile(hLogFile, " (%s:%u)", ShortenPath(entry.FileName, shortPath, sizeof(shortPath)), entry.LineNumber);
			}

			WriteToFile(hLogFile, "\n");
		}

		WriteModules(hLogFile);

		WriteToFile(hLogFile, "----------------------------------------------------------------------------------------------------\n");

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
