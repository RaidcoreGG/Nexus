///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UrlRequests.h
/// Description  :  Receives nexus:// install requests queued by the protocol handler exe.
/// Authors      :  belst
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>
#include <windows.h>

#include "Core/Logging/LogApi.h"
#include "Host/Loader/Loader.h"
#include "LibAddon.h"
#include "LibManager.h"

namespace Raidcore::Nexus::Host
{
	class UrlRequestMgr
	{
		public:
		UrlRequestMgr(
			Core::LogApi&         aLogger,
			LibraryMgr&           aLibrary,
			Loader&               aLoader,
			std::filesystem::path aDirectory
		);

		~UrlRequestMgr();
		
		// unpacks the exe and registers the protocol, or unregisters if disabled
		void Setup(HMODULE aModule, bool aEnabled);
		
		// Drains the queue and starts the listener thread
		void Start();
		void Stop();

		std::vector<LibraryAddon_t> GetPending() const;

		void RemovePending(uint32_t aSignature);

		private:
		Core::LogApi&               Logger;
		LibraryMgr&                 Library;
		Loader&                     Loader;
		std::filesystem::path       HandlerPath;
		std::filesystem::path       QueuePath;

		HANDLE                      RequestEvent{ nullptr };
		HANDLE                      StopEvent{ nullptr };
		std::thread                 ListenerThread;

		mutable std::mutex          Mutex;
		std::vector<LibraryAddon_t> Pending;
		
		// unpacks the exe if changed
		bool UnpackHandler(HMODULE aModule);
		
		// registers the protocol handler
		void Register();
		
		// removes the protocol handler, checks if it's ours (multiple gw2 installations?)
		void Unregister();

		void Listen();

		void Drain();
	};
}
