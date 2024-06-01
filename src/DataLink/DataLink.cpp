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
	void* ADDONAPI_ShareResource(const char* aIdentifier, size_t aResourceSize)
	{
		return ShareResource(aIdentifier, aResourceSize);
	}
}

namespace DataLink
{
	std::mutex										Mutex;
	std::unordered_map<std::string, LinkedResource>	Registry;

	void Free()
	{
		if (State::Nexus == ENexusState::SHUTTING_DOWN)
		{
			const std::lock_guard<std::mutex> lock(Mutex);
			
			while (Registry.size() > 0)
			{
				const auto& it = Registry.begin();

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

				LogInfo(CH_DATALINK, "Freed shared resource: \"%s\"", it->first.c_str());

				Registry.erase(it);
			}
		}
		
	}

	void* GetResource(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		void* result = nullptr;

		const std::lock_guard<std::mutex> lock(Mutex);
		
		const auto& it = Registry.find(str);
		if (it != Registry.end())
		{
			result = it->second.Pointer;
		}

		return result;
	}

	void* ShareResource(const char* aIdentifier, size_t aResourceSize)
	{
		return ShareResource(aIdentifier, aResourceSize, "");
	}
	void* ShareResource(const char* aIdentifier, size_t aResourceSize, const char* aUnderlyingName)
	{
		std::string str = aIdentifier;
		std::string strOverride = aUnderlyingName;

		void* result = nullptr;

		const std::lock_guard<std::mutex> lock(Mutex);
		
		/* resource already exists */
		const auto& it = Registry.find(str);
		if (it != Registry.end())
		{
			if (it->second.Size == aResourceSize)
			{
				return it->second.Pointer;
			}
			else
			{
				LogWarning(CH_DATALINK, "Resource with name \"%s\" already exists with size %u but size %u was requested.", str.c_str(), it->second.Size, aResourceSize);
				return nullptr;
			}
		}

		/* allocate new resource */
		LinkedResource resource{};
		resource.Size = aResourceSize;

		if (strOverride.empty())
		{
			strOverride = str + "_" + std::to_string(GetCurrentProcessId());
		}

		resource.UnderlyingName = strOverride;

		resource.Handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, strOverride.c_str());
		if (!resource.Handle)
		{
			resource.Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, static_cast<DWORD>(aResourceSize), strOverride.c_str());
		}

		if (resource.Handle)
		{
			resource.Pointer = MapViewOfFile(resource.Handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<DWORD>(aResourceSize));

			Registry[str] = resource;
			result = resource.Pointer;
		}

		/* initial null */
		memset(resource.Pointer, 0, resource.Size);

		LogInfo(CH_DATALINK, "Created shared resource: \"%s\" (with underlying name \"%s\")", str.c_str(), strOverride.c_str());

		return result;
	}
}
