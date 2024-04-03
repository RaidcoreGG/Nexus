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
#include <chrono>

#include "Consts.h"
#include "Shared.h"
#include "State.h"
#include "core.h"

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
{
	PVOID Object;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR HandleValue;
	ULONG GrantedAccess;
	USHORT CreatorBackTraceIndex;
	USHORT ObjectTypeIndex;
	ULONG HandleAttributes;
	ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX
{
	ULONG_PTR NumberOfHandles;
	ULONG_PTR Reserved;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, * PSYSTEM_HANDLE_INFORMATION_EX;

typedef NTSTATUS(* PFN_NTQUERYSYSTEMINFORMATION)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID                    SystemInformation,
	ULONG                    SystemInformationLength,
	PULONG                   ReturnLength
);

NTSTATUS GetProcessHandles(std::vector<HANDLE>& handles)
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
			handles.push_back((HANDLE)handle.HandleValue);
		}
	}

	delete[] info;
	return result;
}

typedef NTSTATUS(* PFN_NTQUERYOBJECT)(
	HANDLE                   Handle,
	OBJECT_INFORMATION_CLASS ObjectInformationClass,
	PVOID                    ObjectInformation,
	ULONG                    ObjectInformationLength,
	PULONG                   ReturnLength
);

bool GetHandleName(HANDLE handle, PUNICODE_STRING name, ULONG nameSize)
{
	static PFN_NTQUERYOBJECT func;
	FindFunction(GetModuleHandle("ntdll.dll"), &func, "NtQueryObject");

	NTSTATUS result = func(handle, (OBJECT_INFORMATION_CLASS)0x01, name, nameSize,	&nameSize);
	return NT_SUCCESS(result) ? true : false;
}

bool UnicodeStringContains(PCUNICODE_STRING String1, PCUNICODE_STRING String2)
{
	if (String1->Buffer == nullptr && String2->Buffer == nullptr)
	{
		return true;
	}
	if (String1->Buffer != nullptr && String2->Buffer != nullptr)
	{
		return wcsstr(String1->Buffer, String2->Buffer) != nullptr;
	}
	return false;
}

typedef NTSTATUS(*PFN_RTLINITUNICODESTRING)(
	PUNICODE_STRING DestinationString,
	PCWSTR          SourceString
);

namespace Multibox
{
	void ShareArchive()
	{
		LogCritical(CH_CORE, "Multibox::ShareArchive() not implemented.");
		return;
	}

	void ShareLocal()
	{
		LogCritical(CH_CORE, "Multibox::ShareLocal() not implemented.");
		return;
	}

	void KillMutex()
	{
		bool wasClosed = false;

		auto start_time = std::chrono::high_resolution_clock::now();

		ULONG handleNameSize = 2048;
		UNICODE_STRING* handleName = (UNICODE_STRING*) new unsigned char[handleNameSize];

		UNICODE_STRING mutexName;
		static PFN_RTLINITUNICODESTRING func;
		FindFunction(GetModuleHandle("ntdll.dll"), &func, "RtlInitUnicodeString");
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

		delete[] handleName;

		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;
		if (wasClosed)
		{
			State::MultiboxState |= EMultiboxState::MUTEX_CLOSED;
			LogInfo(CH_CORE, "Destroyed \"AN-Mutex-Window-Guild Wars 2\" in %ums.", time / std::chrono::milliseconds(1));
		}
		else
		{
			LogWarning(CH_CORE, "No mutex was closed.");
		}
	}
}
