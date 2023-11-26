#include "CrashHandler.h"

#include <Windows.h>

typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

void create_minidump(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
    HMODULE mhLib = ::LoadLibrary(_T("dbghelp.dll"));

    if (!mhLib)
    {
        MessageBoxA(NULL, "Could not generate crash dump.", "Unhandled Exception", 0);
        return;
    }

    MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(mhLib, "MiniDumpWriteDump");

    HANDLE hFile = ::CreateFile(_T("nxscrash.dmp"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
    ExInfo.ThreadId = ::GetCurrentThreadId();
    ExInfo.ExceptionPointers = apExceptionInfo;
    ExInfo.ClientPointers = FALSE;

    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
    ::CloseHandle(hFile);
}

LONG WINAPI UnhandledExcHandler(struct _EXCEPTION_POINTERS* aExceptionInfoPtr)
{
    MessageBoxA(NULL, "I'M CRASHING BROOOO.", "Unhandled Exception", 0);
    create_minidump(aExceptionInfoPtr);
    return EXCEPTION_CONTINUE_SEARCH;
}