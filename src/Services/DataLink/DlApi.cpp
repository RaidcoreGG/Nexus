///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlApi.cpp
/// Description  :  Provides functions to share data and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "DlApi.h"

#include <assert.h>

CDataLinkApi::CDataLinkApi(CLogApi* aLogger)
{
	assert(aLogger);

	this->Logger = aLogger;
}

CDataLinkApi::~CDataLinkApi()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	while (this->Registry.size() > 0)
	{
		const auto& it = this->Registry.begin();

		switch (it->second.Type)
		{
			case ELinkedResourceType::Public:
				if (it->second.Pointer)
				{
					UnmapViewOfFile((LPVOID)it->second.Pointer);
					it->second.Pointer = nullptr;
				}

				if (it->second.Handle)
				{
					CloseHandle(it->second.Handle);
					it->second.Handle = nullptr;
				}
				break;

			case ELinkedResourceType::Internal:
				if (it->second.Pointer)
				{
					delete it->second.Pointer;
					it->second.Pointer = nullptr;
				}
				break;
		}

		this->Logger->Info(CH_DATALINK, "Freed shared resource: \"%s\"", it->first.c_str());

		this->Registry.erase(it);
	}
}

void* CDataLinkApi::GetResource(const char* aIdentifier)
{
	if (aIdentifier == nullptr) { return nullptr; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		return it->second.Pointer;
	}

	return nullptr;
}

void* CDataLinkApi::ShareResource(const char* aIdentifier, size_t aResourceSize, const char* aUnderlyingName, bool aIsPublic)
{
	if (aIdentifier == nullptr) { return nullptr; }
	if (aResourceSize == 0)     { return nullptr; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	/* resource already exists */
	if (it != this->Registry.end())
	{
		if (it->second.Size == aResourceSize)
		{
			return it->second.Pointer;
		}
		else /* size mismatch */
		{
			this->Logger->Warning(CH_DATALINK, "Resource with name \"%s\" already exists with size %u but size %u was requested.", aIdentifier, it->second.Size, aResourceSize);
			return nullptr;
		}
	}

	/* allocate new resource */
	LinkedResource resource{};
	resource.Size = aResourceSize;
	resource.Type = aIsPublic ? ELinkedResourceType::Public : ELinkedResourceType::Internal;

	switch (resource.Type)
	{
		default:
		case ELinkedResourceType::None:
		{
			return nullptr;
		}
		case ELinkedResourceType::Public:
		{
			resource.UnderlyingName = aUnderlyingName;

			/* If no name override is set, use identifier + process ID. */
			if (resource.UnderlyingName.empty())
			{
				resource.UnderlyingName = aIdentifier;
				resource.UnderlyingName.append("_");
				resource.UnderlyingName.append(std::to_string(GetCurrentProcessId()));
			}

			/* acquire handle */
			resource.Handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, resource.UnderlyingName.c_str());

			if (!resource.Handle)
			{
				resource.Handle = CreateFileMappingA(
					INVALID_HANDLE_VALUE,
					0,
					PAGE_READWRITE,
					0,
					static_cast<DWORD>(aResourceSize),
					resource.UnderlyingName.c_str()
				);
			}

			/* still no resource handle */
			if (!resource.Handle)
			{
				this->Logger->Warning(
					CH_DATALINK,
					"Failed to create resource \"%s\". OpenFileMapping failed. CreateFileMapping failed. GetLastError: %d",
					aIdentifier,
					GetLastError()
				);
				return nullptr;
			}

			resource.Pointer = MapViewOfFile(resource.Handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<DWORD>(aResourceSize));

			/* sanity check */
			if (!resource.Pointer)
			{
				this->Logger->Warning(
					CH_DATALINK,
					"Failed to create resource \"%s\". MapViewOfFile failed. GetLastError: %d",
					aIdentifier,
					GetLastError()
				);
				return nullptr;
			}

			memset(resource.Pointer, 0, resource.Size);

			this->Logger->Info(CH_DATALINK, "Created public shared resource: \"%s\" (Underlying name: \"%s\")", aIdentifier, resource.UnderlyingName.c_str());
			break;
		}
		case ELinkedResourceType::Internal:
		{
			resource.Pointer = new char[resource.Size];

			this->Logger->Info(CH_DATALINK, "Created internal shared resource: \"%s\"", aIdentifier);
			break;
		}
	}

	/* store linkedresource */
	this->Registry.emplace(aIdentifier, resource);

	return resource.Pointer;
}

std::unordered_map<std::string, LinkedResource> CDataLinkApi::GetRegistry()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}
