///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DataLink.cpp
/// Description  :  Provides functions to share data and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "DataLink.h"

#include "Consts.h"
#include "Shared.h"
#include "State.h"

namespace DataLink
{
	void* ADDONAPI_GetResource(const char* aIdentifier)
	{
		return DataLinkService->GetResource(aIdentifier);
	}

	void* ADDONAPI_ShareResource(const char* aIdentifier, size_t aResourceSize)
	{
		return DataLinkService->ShareResource(aIdentifier, aResourceSize, false);
	}
}

CDataLink::~CDataLink()
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

		Logger->Info(CH_DATALINK, "Freed shared resource: \"%s\"", it->first.c_str());

		this->Registry.erase(it);
	}
}

void* CDataLink::GetResource(const char* aIdentifier)
{
	std::string str = aIdentifier;

	void* result = nullptr;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	const auto& it = this->Registry.find(str);
	if (it != this->Registry.end())
	{
		result = it->second.Pointer;
	}

	return result;
}

void* CDataLink::ShareResource(const char* aIdentifier, size_t aResourceSize, bool aIsPublic)
{
	return ShareResource(aIdentifier, aResourceSize, "", aIsPublic);
}

void* CDataLink::ShareResource(const char* aIdentifier, size_t aResourceSize, const char* aUnderlyingName, bool aIsPublic)
{
	std::string str = aIdentifier;
	std::string strOverride = aUnderlyingName;

	void* result = nullptr;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* resource already exists */
	const auto& it = this->Registry.find(str);
	if (it != this->Registry.end())
	{
		if (it->second.Size == aResourceSize)
		{
			return it->second.Pointer;
		}
		else
		{
			Logger->Warning(CH_DATALINK, "Resource with name \"%s\" already exists with size %u but size %u was requested.", str.c_str(), it->second.Size, aResourceSize);
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
		return nullptr;
	case ELinkedResourceType::Public:
		/* attach PID for unique name */
		if (strOverride.empty())
		{
			strOverride = str + "_" + std::to_string(GetCurrentProcessId());
		}

		resource.UnderlyingName = strOverride;

		/* acquire handle */
		resource.Handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, strOverride.c_str());
		if (!resource.Handle)
		{
			resource.Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, static_cast<DWORD>(aResourceSize), strOverride.c_str());
		}

		/* acquire pointer */
		if (resource.Handle)
		{
			resource.Pointer = MapViewOfFile(resource.Handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<DWORD>(aResourceSize));

			/* sanity check */
			if (!resource.Pointer)
			{
				return nullptr;
			}
		}

		Logger->Info(CH_DATALINK, "Created public shared resource: \"%s\" (with underlying name \"%s\")", str.c_str(), strOverride.c_str());
		break;

	case ELinkedResourceType::Internal:
		resource.Pointer = new char[resource.Size];

		Logger->Info(CH_DATALINK, "Created internal shared resource: \"%s\"", str.c_str());
		break;
	}

	/* store linkedresource */
	this->Registry[str] = resource;
	result = resource.Pointer;

	/* initial null */
	memset(resource.Pointer, 0, resource.Size);

	return result;
}

std::unordered_map<std::string, LinkedResource> CDataLink::GetRegistry()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}
