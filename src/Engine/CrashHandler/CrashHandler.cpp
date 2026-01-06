///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CrashHandler.cpp
/// Description  :  Unhandled exception crash logger.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CrashHandler.h"

#include <dbghelp.h>
#include <shellapi.h>

static CCrashHandler* s_CrashHandler{};
static PVOID s_VectoredHandlerHandle = nullptr;

CCrashHandler::CCrashHandler(std::filesystem::path aLogPath)
{
	GetSystemInfo(&this->SystemInfo);

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

	this->VEH = AddVectoredExceptionHandler(1, CCrashHandler::OnVectoredException);
	this->UEF = SetUnhandledExceptionFilter(CCrashHandler::OnUnhandledException);
	
	this->LogPath = aLogPath;
	this->LogStream = std::ofstream{this->LogPath, std::ios_base::out, SH_DENYWR};

	if (s_CrashHandler)
	{
		throw "CCrashHandler already registered.";
	}

	s_CrashHandler = this;
}

CCrashHandler::~CCrashHandler()
{
	this->LogStream.close();

	if (this->VEH)
	{
		RemoveVectoredExceptionHandler(this->VEH);
	}

	if (this->UEF)
	{
		SetUnhandledExceptionFilter(this->UEF);
	}
}

/*static*/ LONG WINAPI CCrashHandler::OnVectoredException(EXCEPTION_POINTERS* aExcPointers)
{
	DWORD code = aExcPointers->ExceptionRecord->ExceptionCode;

	switch (code)
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
			OnUnhandledException(aExcPointers);
			break;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


/*static*/ LONG WINAPI CCrashHandler::OnUnhandledException(EXCEPTION_POINTERS* aExcPointers)
{
	HANDLE hProcess = GetCurrentProcess();

	if (!SymInitialize(hProcess, NULL, TRUE))
	{
		SymCleanup(hProcess);
		SymInitialize(hProcess, NULL, TRUE);
	}

	std::vector<FunctionCall> callstack{};

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
	HANDLE hThread = GetCurrentThread();
	while (StackWalk64(s_CrashHandler->MachineType, hProcess, hThread, &sf, aExcPointers->ContextRecord, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0))
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

		FunctionCall currentCall{};

		IMAGEHLP_MODULE64 moduleInfo{};
		moduleInfo.SizeOfStruct = sizeof(moduleInfo);

		if (SymGetModuleInfo64(hProcess, sf.AddrPC.Offset, &moduleInfo))
		{
			currentCall.ModuleOffset = sf.AddrPC.Offset - moduleInfo.BaseOfImage;
		}

		DWORD64 dwSymDisplacement = 0;
		if (SymFromAddr(hProcess, sf.AddrPC.Offset, &dwSymDisplacement, pSymbol))
		{
			currentCall.FunctionName = pSymbol->Name;
		}
		else
		{
			/* No symbol, so we log the module offset. */
			currentCall.FunctionName = std::format("{:#x}", currentCall.ModuleOffset);
		}

		/* Get line and module name. */
		IMAGEHLP_LINE64 lineInfo{};
		DWORD dwLineDisplacement = 0;

		if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo))
		{
			currentCall.FileName = lineInfo.FileName;
			currentCall.LineNumber = lineInfo.LineNumber;
		}
		else
		{
			currentCall.FileName = moduleInfo.ImageName;
			currentCall.LineNumber = -1;
		}

		/* Store information. */
		callstack.push_back(currentCall);
	}

	/* Cleanup. */
	//SymCleanup(hProcess);

	if (!s_CrashHandler->LogStream.is_open())
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	s_CrashHandler->LogStream << "========================\n";
	s_CrashHandler->LogStream << "Exception:" << std::hex << aExcPointers->ExceptionRecord->ExceptionCode << "\n";
	s_CrashHandler->LogStream << "========================\n";
	s_CrashHandler->LogStream << "Call Stack Dump:\n";
	s_CrashHandler->LogStream << "========================\n";

	size_t frameIndex = 0;
	for (const auto& frame : callstack)
	{
		s_CrashHandler->LogStream << std::setw(3) << frameIndex++ << ": "
			<< frame.FileName << "@" << frame.FunctionName;

		if (frame.LineNumber != -1)
		{
			s_CrashHandler->LogStream << " on line " << frame.LineNumber;
		}

		s_CrashHandler->LogStream << "\n";
	}

	s_CrashHandler->LogStream << "========================\n\n";

	s_CrashHandler->LogStream.flush();

	//MessageBoxA(0, "Opening log.", "UEF", 0);

	ShellExecuteA(nullptr, "open", s_CrashHandler->LogPath.string().c_str(), nullptr, nullptr, SW_SHOWNORMAL);

	return EXCEPTION_EXECUTE_HANDLER;
}
