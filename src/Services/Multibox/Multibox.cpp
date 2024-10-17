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

#include "Util/Strings.h"
#include "Util/DLL.h"
#include "Util/CmdLine.h"

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
		DLL::FindFunction(GetModuleHandleA("ntdll.dll"), &func, "NtQuerySystemInformation");
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
	DLL::FindFunction(GetModuleHandleA("ntdll.dll"), &func, "NtQueryObject");

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

EMultiboxState operator|(EMultiboxState lhs, EMultiboxState rhs)
{
	return static_cast<EMultiboxState>(std::underlying_type_t<EMultiboxState>(lhs) | std::underlying_type_t<EMultiboxState>(rhs));
}
EMultiboxState operator&(EMultiboxState lhs, EMultiboxState rhs)
{
	return static_cast<EMultiboxState>(std::underlying_type_t<EMultiboxState>(lhs) & std::underlying_type_t<EMultiboxState>(rhs));
}
EMultiboxState operator|=(EMultiboxState& lhs, EMultiboxState rhs)
{
	return lhs = lhs | rhs;
}

namespace Multibox
{
	static EMultiboxState s_State          = EMultiboxState::NONE;
	static bool           s_ArchiveShared  = false;
	static bool           s_LocalShared    = false;
	static bool           s_MutexDestroyed = false;

	EMultiboxState GetState()
	{
		s_State = EMultiboxState::NONE;

		if (s_ArchiveShared)  { s_State |= EMultiboxState::ARCHIVE_SHARED; }
		if (s_LocalShared)    { s_State |= EMultiboxState::LOCAL_SHARED; }
		if (s_MutexDestroyed) { s_State |= EMultiboxState::MUTEX_CLOSED; }

		return s_State;
	}

	void ShareArchive()
	{
		if (CmdLine::HasArgument("-sharearchive"))
		{
			s_ArchiveShared = true;
		}

		//Logger->Critical(CH_CORE, "Multibox::ShareArchive() not implemented.");
	}

	void ShareLocal()
	{
		if (CmdLine::HasArgument("-multi"))
		{
			s_LocalShared = true;
		}

		//Logger->Critical(CH_CORE, "Multibox::ShareLocal() not implemented.");
	}

	void KillMutex()
	{
		bool wasClosed = false;

		ULONG handleNameSize = 2048;
		UNICODE_STRING* handleName = (UNICODE_STRING*) new unsigned char[handleNameSize];

		UNICODE_STRING mutexName;
		PFN_RTLINITUNICODESTRING func;
		if (DLL::FindFunction(GetModuleHandleA("ntdll.dll"), &func, "RtlInitUnicodeString"))
		{
			func(&mutexName, L"AN-Mutex-Window-Guild Wars 2");

			std::vector<HANDLE> handles;
			GetProcessHandles(handles);

			bool found = false;

			for (int i = 0; i < handles.size(); i++)
			{
				HANDLE handle = handles[i];
				if (GetHandleName(handle, handleName, handleNameSize))
				{
					if (UnicodeStringContains(handleName, &mutexName))
					{
						found = true;

						if (CloseHandle(handle))
						{
							s_MutexDestroyed = true;
						}

						break;
					}
				}
			}

			if (!found)
			{
				s_MutexDestroyed = true;
			}
		}

		delete[] handleName;
	}
}
