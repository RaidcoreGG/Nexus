///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UrlRequests.cpp
/// Description  :  Receives nexus:// install requests queued by the protocol handler exe.
/// Authors      :  belst
///----------------------------------------------------------------------------------------------------

#include "UrlRequests.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <string>
#include <string_view>

#include "res/ResConst.h"
#include "Util/Resources.h"
#include "UrlHandler/UrlProtocol.h"

namespace Raidcore::Nexus::Host
{
	constexpr const char*    LOG_CHANNEL     = "URL";

	constexpr const wchar_t* KEY_PROTOCOL    = L"Software\\Classes\\nexus";
	constexpr const wchar_t* KEY_COMMAND     = L"Software\\Classes\\nexus\\shell\\open\\command";

	static std::wstring GetRegisteredCommand()
	{
		wchar_t buffer[1024];
		DWORD size = sizeof(buffer);

		// ERROR_MORE_DATA should not happen, path limit is shorter than buffer size
		if (RegGetValueW(HKEY_CURRENT_USER, KEY_COMMAND, nullptr, RRF_RT_REG_SZ, nullptr, buffer, &size) != ERROR_SUCCESS)
		{
			return {};
		}

		/* size includes the terminator. */
		return std::wstring(buffer, size / sizeof(wchar_t) - 1);
	}

	static bool SetRegistryString(const wchar_t* aKey, const wchar_t* aName, const std::wstring& aValue)
	{
		DWORD bytes = static_cast<DWORD>((aValue.size() + 1) * sizeof(wchar_t));
		return RegSetKeyValueW(HKEY_CURRENT_USER, aKey, aName, REG_SZ, aValue.c_str(), bytes) == ERROR_SUCCESS;
	}

	UrlRequestMgr::UrlRequestMgr(
		Core::LogApi&         aLogger,
		LibraryMgr&           aLibrary,
		Host::Loader&         aLoader,
		std::filesystem::path aDirectory
	)
		: Logger(aLogger)
		, Library(aLibrary)
		, Loader(aLoader)
		, HandlerPath(aDirectory / UrlProtocol::HANDLER_FILENAME)
		, QueuePath(aDirectory / UrlProtocol::QUEUE_FILENAME)
	{}

	UrlRequestMgr::~UrlRequestMgr()
	{
		this->Stop();
	}

	void UrlRequestMgr::SetEnabled(HMODULE aModule, bool aEnabled)
	{
		const std::lock_guard<std::mutex> lock(this->LifecycleMutex);

		this->Setup(aModule, aEnabled);

		if (aEnabled)
		{
			this->Start();
		}
		else
		{
			this->Stop();
		}
	}

	void UrlRequestMgr::Setup(HMODULE aModule, bool aEnabled)
	{
		if (!aEnabled)
		{
			this->Unregister();
			return;
		}

		if (this->UnpackHandler(aModule))
		{
			this->Register();
		}
	}

	void UrlRequestMgr::Start()
	{
		if (this->ListenerThread.joinable()) { return; }

		// Auto reset, to prevent multiple installs while multiboxing
		this->RequestEvent = CreateEventA(nullptr, false, false, UrlProtocol::EVENT_REQUEST);
		this->StopEvent = CreateEventA(nullptr, true, false, nullptr);

		if (!this->RequestEvent || !this->StopEvent)
		{
			this->Logger.Warning(LOG_CHANNEL, "Failed to create events. URL requests disabled. Error: %u", GetLastError());
			this->Stop();
			return;
		}

		this->ListenerThread = std::thread(&UrlRequestMgr::Listen, this);
	}

	void UrlRequestMgr::Stop()
	{
		if (this->StopEvent)
		{
			SetEvent(this->StopEvent);
		}

		if (this->ListenerThread.joinable())
		{
			this->ListenerThread.join();
		}

		if (this->RequestEvent)
		{
			CloseHandle(this->RequestEvent);
			this->RequestEvent = nullptr;
		}

		if (this->StopEvent)
		{
			CloseHandle(this->StopEvent);
			this->StopEvent = nullptr;
		}
	}

	std::vector<LibraryAddon_t> UrlRequestMgr::GetPending() const
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		return this->Pending;
	}

	void UrlRequestMgr::RemovePending(uint32_t aSignature)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		std::erase_if(this->Pending, [aSignature](const LibraryAddon_t& aAddon) { return aAddon.Signature == aSignature; });
	}

	bool UrlRequestMgr::UnpackHandler(HMODULE aModule)
	{
		LPVOID res{};
		DWORD size{};

		if (!Resources::Get(aModule, MAKEINTRESOURCEA(RES_URLHANDLER), "EXE", &res, &size))
		{
			this->Logger.Warning(LOG_CHANNEL, "Embedded URL handler not found.");
			return false;
		}

		std::string_view embedded(static_cast<const char*>(res), size);

		try
		{
			// don't write if no change, exe may be running
			if (std::filesystem::exists(this->HandlerPath) && std::filesystem::file_size(this->HandlerPath) == size)
			{
				std::ifstream in(this->HandlerPath, std::ios::binary);
				std::string existing(size, '\0');
				in.read(existing.data(), size);

				if (in && existing == embedded) { return true; }
			}
			
			// atomic tmp swap
			std::filesystem::path tmpPath = this->HandlerPath;
			tmpPath += ".tmp";

			std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
			out.write(embedded.data(), embedded.size());

			// close and flush so the rename works
			out.close();

			if (!out)
			{
				this->Logger.Warning(LOG_CHANNEL, "Failed to write \"%s\".", tmpPath.string().c_str());
				return false;
			}

			std::filesystem::rename(tmpPath, this->HandlerPath);
		}
		catch (const std::exception& ex)
		{
			this->Logger.Warning(LOG_CHANNEL, "Failed to unpack URL handler: %s", ex.what());

			// return true if old version exists.
			// ec overload to avoid throwing inside the catch handler
			std::error_code ec;
			return std::filesystem::exists(this->HandlerPath, ec);
		}

		this->Logger.Info(LOG_CHANNEL, "Unpacked URL handler to \"%s\".", this->HandlerPath.string().c_str());
		return true;
	}

	void UrlRequestMgr::Register()
	{
		std::wstring command = L"\"" + this->HandlerPath.wstring() + L"\" \"%1\"";

		if (GetRegisteredCommand() == command) { return; }

		bool success = SetRegistryString(KEY_PROTOCOL, nullptr, L"URL:Nexus Protocol")
			&& SetRegistryString(KEY_PROTOCOL, L"URL Protocol", L"")
			&& SetRegistryString(KEY_COMMAND, nullptr, command);

		if (!success)
		{
			this->Logger.Warning(LOG_CHANNEL, "Failed to register nexus:// protocol.");
			return;
		}

		this->Logger.Info(LOG_CHANNEL, "Registered nexus:// protocol.");
	}

	void UrlRequestMgr::Unregister()
	{
		std::wstring command = L"\"" + this->HandlerPath.wstring() + L"\" \"%1\"";

		if (GetRegisteredCommand() != command) { return; }

		if (RegDeleteTreeW(HKEY_CURRENT_USER, KEY_PROTOCOL) != ERROR_SUCCESS)
		{
			this->Logger.Warning(LOG_CHANNEL, "Failed to remove nexus:// protocol.");
			return;
		}

		this->Logger.Info(LOG_CHANNEL, "Removed nexus:// protocol.");
	}

	void UrlRequestMgr::Listen()
	{
		// Initial install of requests, while the game was closed
		this->Drain();

		HANDLE handles[] = { this->StopEvent, this->RequestEvent };
		
		// Block until request event or stop, only drain on request events
		while (WaitForMultipleObjects(std::size(handles), handles, false, INFINITE) == WAIT_OBJECT_0 + 1)
		{
			this->Drain();
		}
	}

	void UrlRequestMgr::Drain()
	{
		std::vector<LibraryAddon_t> library = this->Library.GetLibrary();
		
		// Library is empty, check logs for errors
		if (library.empty())
		{
			this->Logger.Info(LOG_CHANNEL, "Addon library unavailable. Leaving URL requests queued.");
			return;
		}

		std::vector<UrlProtocol::QueueEntry_t> queue;

		HANDLE mutex = CreateMutexA(nullptr, false, UrlProtocol::MUTEX_QUEUE);

		if (!mutex)
		{
			this->Logger.Warning(LOG_CHANNEL, "Failed to open queue mutex. Error: %u", GetLastError());
			return;
		}

		DWORD wait = WaitForSingleObject(mutex, 5000);

		if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED)
		{
			CloseHandle(mutex);
			this->Logger.Warning(LOG_CHANNEL, "Timed out waiting for queue mutex.");
			return;
		}

		try
		{
			std::ifstream in(this->QueuePath);

			if (in)
			{
				queue = json::parse(in).get<std::vector<UrlProtocol::QueueEntry_t>>();
			}
		}
		catch (...)
		{
			this->Logger.Warning(LOG_CHANNEL, "Error parsing URL request queue.");
		}

		// ec for noexcept, something went wrong, but not wrong enough to crash the game
		std::error_code ec;
		if (!std::filesystem::remove(this->QueuePath, ec) && ec)
		{
			this->Logger.Warning(LOG_CHANNEL, "Failed to delete URL request queue: %s", ec.message().c_str());
		}

		ReleaseMutex(mutex);
		CloseHandle(mutex);

		for (const UrlProtocol::QueueEntry_t& entry : queue)
		{
			if (this->Loader.IsTrackedSafe(entry.Id))
			{
				this->Logger.Info(LOG_CHANNEL, "Ignoring install request for 0x%08X. Already installed.", entry.Id);
				continue;
			}

			auto it = std::ranges::find(library, entry.Id, &LibraryAddon_t::Signature);

			if (it == library.end())
			{
				this->Logger.Warning(LOG_CHANNEL, "Ignoring install request for 0x%08X. Not in the addon library.", entry.Id);
				continue;
			}

			const std::lock_guard<std::mutex> lock(this->Mutex);
			
			// Already pending, ignore
			if (std::ranges::find(this->Pending, entry.Id, &LibraryAddon_t::Signature) != this->Pending.end()) { continue; }

			this->Pending.push_back(*it);

			this->Logger.Info(LOG_CHANNEL, "Install request for \"%s\" (0x%08X) awaiting confirmation.", it->Name.c_str(), it->Signature);
		}
	}
}
