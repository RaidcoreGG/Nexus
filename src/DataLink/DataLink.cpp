#include "DataLink.h"

#include "../Shared.h"

namespace DataLink
{
	std::mutex DataLinkMutex;
	std::map<std::string, LinkedResource> DataLinkRegistry;

	void Shutdown()
	{
		DataLinkMutex.lock();
		
		while (DataLinkRegistry.size() > 0)
		{
			const auto& it = DataLinkRegistry.begin();

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

			LogDebug("Freed shared resource: %s", it->first.c_str());

			DataLinkRegistry.erase(it);
		}
		DataLinkMutex.unlock();
	}

	void* GetResource(std::string aIdentifier)
	{
		DataLinkMutex.lock();

		void* result = nullptr;

		if (DataLinkRegistry.find(aIdentifier) != DataLinkRegistry.end())
		{
			result = DataLinkRegistry[aIdentifier].Pointer;
		}

		DataLinkMutex.unlock();

		return result;
	}

	void* ShareResource(std::string aIdentifier, size_t aResourceSize)
	{
		DataLinkMutex.lock();

		/* resource already exists */
		if (DataLinkRegistry.find(aIdentifier) != DataLinkRegistry.end())
		{
			DataLinkMutex.unlock();
			return DataLinkRegistry[aIdentifier].Pointer;
		}

		/* allocate new resource */
		LinkedResource resource{};
		resource.Size = aResourceSize;

		resource.Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, aResourceSize, ("ADDONHOST_" + aIdentifier).c_str());

		if (resource.Handle)
		{
			resource.Pointer = MapViewOfFile(resource.Handle, FILE_MAP_ALL_ACCESS, 0, 0, aResourceSize);

			DataLinkRegistry[aIdentifier] = resource;

			DataLinkMutex.unlock();
			return resource.Pointer;
		}

		DataLinkMutex.unlock();

		return nullptr;
	}
}