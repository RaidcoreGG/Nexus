#include "DataLink.h"

namespace DataLink
{
	std::mutex								Mutex;
	std::map<std::string, LinkedResource>	Registry;

	void Free()
	{
		Mutex.lock();
		{
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

				LogDebug(CH_DATALINK, "Freed shared resource: \"%s\"", it->first.c_str());

				Registry.erase(it);
			}
		}
		Mutex.unlock();
	}

	void* GetResource(std::string aIdentifier)
	{
		void* result = nullptr;

		Mutex.lock();
		{
			if (Registry.find(aIdentifier) != Registry.end())
			{
				result = Registry[aIdentifier].Pointer;
			}
		}
		Mutex.unlock();

		return result;
	}

	void* ShareResource(std::string aIdentifier, size_t aResourceSize)
	{
		return ShareResource(aIdentifier, aResourceSize, "");
	}
	void* ShareResource(std::string aIdentifier, size_t aResourceSize, std::string aResourceNameOverride)
	{
		void* result = nullptr;

		Mutex.lock();
		{
			/* resource already exists */
			if (Registry.find(aIdentifier) != Registry.end())
			{
				result = Registry[aIdentifier].Pointer;
			}
			else
			{
				/* allocate new resource */
				LinkedResource resource{};
				resource.Size = aResourceSize;

				resource.Handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, aResourceNameOverride != "" ? aResourceNameOverride.c_str() : aIdentifier.c_str());
				if (resource.Handle == 0)
				{
					resource.Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, aResourceSize, aResourceNameOverride != "" ? aResourceNameOverride.c_str() : aIdentifier.c_str());
				}

				if (resource.Handle)
				{
					resource.Pointer = MapViewOfFile(resource.Handle, FILE_MAP_ALL_ACCESS, 0, 0, aResourceSize);

					Registry[aIdentifier] = resource;
					result = resource.Pointer;
				}
			}
		}
		Mutex.unlock();

		if (aResourceNameOverride != "")
		{
			LogDebug(CH_DATALINK, "Created shared resource: \"%s\" (with underlying name \"%s\")", aIdentifier.c_str(), aResourceNameOverride.c_str());
		}
		else
		{
			LogDebug(CH_DATALINK, "Created shared resource: \"%s\"", aIdentifier.c_str());
		}

		return result;
	}
}