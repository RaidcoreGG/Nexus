#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <windows.h>
#include <Dbghelp.h>
#include <tchar.h>

LONG WINAPI UnhandledExcHandler(struct _EXCEPTION_POINTERS* aExceptionInfoPtr);

#endif