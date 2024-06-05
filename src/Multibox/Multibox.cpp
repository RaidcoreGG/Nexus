///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Multibox.cpp
/// Description  :  Provides functions to enable multiboxing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Multibox.h"

#define UMDF_USING_NTSTATUS
#include <Windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <vector>
#include <string>

#include "core.h"
#include "Util/Strings.h"

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
{
	PVOID								Object;
	ULONG_PTR							UniqueProcessId;
	ULONG_PTR							HandleValue;
	ULONG								GrantedAccess;
	USHORT								CreatorBackTraceIndex;
	USHORT								ObjectTypeIndex;
	ULONG								HandleAttributes;
	ULONG								Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX
{
	ULONG_PTR							NumberOfHandles;
	ULONG_PTR							Reserved;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX	Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef NTSTATUS(* PFN_NTQUERYSYSTEMINFORMATION)
(
	SYSTEM_INFORMATION_CLASS			SystemInformationClass,
	PVOID								SystemInformation,
	ULONG								SystemInformationLength,
	PULONG								ReturnLength
);

typedef NTSTATUS(*PFN_NTQUERYOBJECT)(
	HANDLE								Handle,
	OBJECT_INFORMATION_CLASS			ObjectInformationClass,
	PVOID								ObjectInformation,
	ULONG								ObjectInformationLength,
	PULONG								ReturnLength
);

typedef NTSTATUS(*PFN_RTLINITUNICODESTRING)(
	PUNICODE_STRING						DestinationString,
	PCWSTR								SourceString
);

NTSTATUS GetProcessHandles(std::vector<HANDLE>& aHandles)
{
	NTSTATUS result;
	ULONG bufferSize = 2048;
	SYSTEM_HANDLE_INFORMATION_EX* info = nullptr;

	do
	{
		delete[] info;
		info = (SYSTEM_HANDLE_INFORMATION_EX*) new BYTE[bufferSize];

		static PFN_NTQUERYSYSTEMINFORMATION func;
		FindFunction(GetModuleHandle("ntdll.dll"), &func, "NtQuerySystemInformation");
		result = func((SYSTEM_INFORMATION_CLASS)0x40, info, bufferSize, &bufferSize);
	}
	while (result == STATUS_INFO_LENGTH_MISMATCH);

	auto currentProcessId = GetCurrentProcessId();
	for (auto i = 0; i < info->NumberOfHandles; i++)
	{
		auto handle = info->Handles[i];
		if (handle.UniqueProcessId == currentProcessId)
		{
			aHandles.push_back((HANDLE)handle.HandleValue);
		}
	}

	delete[] info;

	return result;
}

bool GetHandleName(HANDLE aHandle, PUNICODE_STRING aName, ULONG aNameSize)
{
	static PFN_NTQUERYOBJECT func;
	FindFunction(GetModuleHandle("ntdll.dll"), &func, "NtQueryObject");

	NTSTATUS result = func(aHandle, (OBJECT_INFORMATION_CLASS)0x01, aName, aNameSize, &aNameSize);
	return NT_SUCCESS(result)
		? true
		: false;
}

bool UnicodeStringContains(PCUNICODE_STRING aStr1, PCUNICODE_STRING aStr2)
{
	if (!aStr1->Buffer && !aStr2->Buffer)
	{
		return true;
	}

	if (aStr1->Buffer && aStr2->Buffer)
	{
		return wcsstr(aStr1->Buffer, aStr2->Buffer) != nullptr;
	}

	return false;
}

namespace Multibox
{
	bool ShareArchive()
	{
		std::string cmdLine = String::ToLower(GetCommandLineA());
		if (String::Contains(cmdLine, "-sharearchive"))
		{
			/* archive already shared via commandline arguments */
			return true;
		}

		//LogCritical(CH_CORE, "Multibox::ShareArchive() not implemented.");

		return false;
	}

	bool ShareLocal()
	{
		std::string cmdLine = String::ToLower(GetCommandLineA());
		if (String::Contains(cmdLine, "-multi"))
		{
			/* local archive already shared via commandline arguments */
			return true;
		}

		//LogCritical(CH_CORE, "Multibox::ShareLocal() not implemented.");

		return false;
	}

	bool KillMutex()
	{
		bool wasClosed = false;

		ULONG handleNameSize = 2048;
		UNICODE_STRING* handleName = (UNICODE_STRING*) new unsigned char[handleNameSize];

		UNICODE_STRING mutexName;
		PFN_RTLINITUNICODESTRING func;
		if (FindFunction(GetModuleHandle("ntdll.dll"), &func, "RtlInitUnicodeString"))
		{
			func(&mutexName, L"AN-Mutex-Window-Guild Wars 2");

			std::vector<HANDLE> handles;
			GetProcessHandles(handles);

			for (int i = 0; i < handles.size(); i++)
			{
				HANDLE handle = handles[i];
				if (GetHandleName(handle, handleName, handleNameSize) && UnicodeStringContains(handleName, &mutexName))
				{
					CloseHandle(handle);
					wasClosed = true;
					break;
				}
			}
		}

		delete[] handleName;

		return wasClosed;
	}
}
