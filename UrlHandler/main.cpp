///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  main.cpp
/// Description  :  nexus:// protocol handler. Queues install requests for Nexus to pick up.
/// Authors      :  belst
///----------------------------------------------------------------------------------------------------

#include <windows.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;

#include "UrlProtocol.h"

namespace UrlProtocol = Raidcore::Nexus::UrlProtocol;

enum EExitCode
{
	OK          = 0,
	BAD_ARGS    = 1,
	BAD_URL     = 2,
	NO_LOCK     = 3,
	WRITE_FAIL  = 4
};

static std::filesystem::path GetModuleDirectory()
{
	std::wstring buffer(MAX_PATH, L'\0');

	for (;;)
	{
		DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

		if (length == 0) { return {}; }

		if (length < buffer.size())
		{
			buffer.resize(length);
			return std::filesystem::path(buffer).parent_path();
		}

		buffer.resize(buffer.size() * 2);
	}
}

// called while holding the queue mutex
static bool AppendToQueue(const std::filesystem::path& aPath, uint32_t aId)
{
	std::vector<UrlProtocol::QueueEntry_t> queue;

	// missing file, malformed json or other errors will reset the queue
	try
	{
		std::ifstream in(aPath);

		if (in)
		{
			queue = json::parse(in).get<std::vector<UrlProtocol::QueueEntry_t>>();
		}
	}
	catch (...)
	{
		queue.clear();
	}

	// re-queued id moves to the back
	std::erase_if(queue, [aId](const UrlProtocol::QueueEntry_t& aEntry) { return aEntry.Id == aId; });

	queue.push_back({ aId, static_cast<int64_t>(std::time(nullptr)) });

	// front holds the oldest
	if (queue.size() > UrlProtocol::QUEUE_MAX)
	{
		queue.erase(queue.begin(), queue.end() - UrlProtocol::QUEUE_MAX);
	}

	// Write to a temp file and swap it in to prevent racy reads
	std::filesystem::path tmpPath = aPath;
	tmpPath += ".tmp";

	{
		std::ofstream out(tmpPath, std::ios::trunc);

		if (!out) { return false; }

		out << json(queue).dump();

		if (!out) { return false; }
	}

	return MoveFileExW(tmpPath.c_str(), aPath.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2) { return BAD_ARGS; }

	std::optional<uint32_t> id = UrlProtocol::ParseInstallUrl(argv[1]);

	if (!id) { return BAD_URL; }

	std::filesystem::path queuePath = GetModuleDirectory() / UrlProtocol::QUEUE_FILENAME;

	HANDLE mutex = CreateMutexA(nullptr, false, UrlProtocol::MUTEX_QUEUE);

	if (!mutex) { return NO_LOCK; }

	// Queue Mutex
	DWORD wait = WaitForSingleObject(mutex, 5000);

	if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
	{
		CloseHandle(mutex);
		return NO_LOCK;
	}

	bool written = AppendToQueue(queuePath, *id);

	ReleaseMutex(mutex);
	CloseHandle(mutex);

	if (!written) { return WRITE_FAIL; }
	
	// Send event to nexus so a running instance can immediately process the queue
	HANDLE event = OpenEventA(EVENT_MODIFY_STATE, false, UrlProtocol::EVENT_REQUEST);

	if (event)
	{
		SetEvent(event);
		CloseHandle(event);
	}

	return OK;
}
