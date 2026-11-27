///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbApi.h
/// Description  :  Provides functions for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <windows.h>

#include "Engine/Inputs/RawInput/WndProcNotify.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/Events/EvtApi.h"
#include "IbNotify.h"
#include "Core/Logging/LogChannel.h"
#include "IbBind.h"
#include "IbCapture.h"
#include "IbMapping.h"

namespace Raidcore::Engine::Input
{
	///----------------------------------------------------------------------------------------------------
	/// CInputBindApi Class
	///----------------------------------------------------------------------------------------------------
	class CInputBindApi
		: public Core::ILogChannel
		, public CInputBindCapture
		, public virtual Engine::IRefCleaner
		, public IWndProcNotifyHandler
		, public IInputBindUpdateNotifier
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		CInputBindApi(Engine::CEventApi* aEventApi, std::filesystem::path aConfigPath);

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~CInputBindApi();

		///----------------------------------------------------------------------------------------------------
		/// OnWndProcNotify:
		/// 	Returns 0 if a Bind_t was invoked.
		///----------------------------------------------------------------------------------------------------
		UINT OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam) override;

		///----------------------------------------------------------------------------------------------------
		/// Register:
		/// 	Generates and registers a Bind_t from the given string with the given identifier and handler,
		/// 	if no bind was previously stored the given one will be used.
		///----------------------------------------------------------------------------------------------------
		void Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, const char* aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// Register:
		/// 	Registers a Bind_t from the given struct with the given identifier and handler,
		/// 	if no bind was previously stored the given one will be used.
		///----------------------------------------------------------------------------------------------------
		void Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, Bind_t aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// Deregister:
		/// 	Deregisters a InputBindHandler from an identifier.
		///----------------------------------------------------------------------------------------------------
		void Deregister(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// IsInUse:
		/// 	Returns an empty string if Bind_t is unused or the identifier that uses this Bind_t.
		///----------------------------------------------------------------------------------------------------
		std::string IsInUse(const Bind_t& aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// HasHandler:
		/// 	Returns true if the Bind_t with the passed identifier has a handler registered.
		///----------------------------------------------------------------------------------------------------
		bool HasHandler(const std::string& aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// Get:
		/// 	Gets a InputBind.
		/// 	Returns nullptr, if it doesn't exist.
		///----------------------------------------------------------------------------------------------------
		const Bind_t* Get(const std::string& aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// Set:
		/// 	Sets a InputBind.
		///----------------------------------------------------------------------------------------------------
		void Set(std::string aIdentifier, Bind_t aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// SetPassthrough:
		/// 	Sets the passthrough state of an Bind_t.
		///----------------------------------------------------------------------------------------------------
		void SetPassthrough(std::string aIdentifier, bool aPassthrough);

		///----------------------------------------------------------------------------------------------------
		/// Invoke:
		/// 	Invokes the action on the corresponding Bind_t handler.
		/// 	Returns true if the Bind_t was dispatched and processing should not continue.
		///----------------------------------------------------------------------------------------------------
		bool Invoke(std::string aIdentifier, bool aIsRelease = false);

		///----------------------------------------------------------------------------------------------------
		/// Deletes:
		/// 	Deletes a Bind_t entirely.
		///----------------------------------------------------------------------------------------------------
		void Delete(std::string aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// CleanupRefs:
		/// 	Removes all InputBindHandlers that are within the provided address space.
		///----------------------------------------------------------------------------------------------------
		uint32_t CleanupRefs(void* aStartAddress, void* aEndAddress) override;

		///----------------------------------------------------------------------------------------------------
		/// GetRegistry:
		/// 	Returns a copy of the registry.
		///----------------------------------------------------------------------------------------------------
		std::map<std::string, IbMapping_t> GetRegistry() const;

		private:
		Engine::CEventApi* EventApi = nullptr;

		std::filesystem::path              ConfigPath;

		mutable std::mutex                 Mutex;
		std::map<std::string, IbMapping_t> Registry;
		std::map<std::string, Bind_t> HeldInputBinds;

		///----------------------------------------------------------------------------------------------------
		/// LoadSafe:
		/// 	Loads the InputBinds. Threadafe.
		///----------------------------------------------------------------------------------------------------
		void LoadSafe();

		///----------------------------------------------------------------------------------------------------
		/// Load:
		/// 	Loads the InputBinds. Not threadsafe.
		///----------------------------------------------------------------------------------------------------
		void Load();

		///----------------------------------------------------------------------------------------------------
		/// SaveSafe:
		/// 	Saves the InputBinds. Threadafe.
		///----------------------------------------------------------------------------------------------------
		void SaveSafe();

		///----------------------------------------------------------------------------------------------------
		/// Save:
		/// 	Saves the InputBinds. Not threadsafe.
		///----------------------------------------------------------------------------------------------------
		void Save();

		///----------------------------------------------------------------------------------------------------
		/// Press:
		/// 	Invokes an Bind_t that matches the pressed inputs.
		/// 	Returns true if a bind was found and invoked.
		///----------------------------------------------------------------------------------------------------
		bool Press(const Bind_t& aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// Release:
		/// 	Releases all InputBinds matching the criteria and invokes a release.
		///----------------------------------------------------------------------------------------------------
		void Release(unsigned int aModifierVK);

		///----------------------------------------------------------------------------------------------------
		/// Release:
		/// 	Releases all InputBinds matching the criteria and invokes a release.
		///----------------------------------------------------------------------------------------------------
		void Release(EDevice aDevice, unsigned short aCode);

		///----------------------------------------------------------------------------------------------------
		/// ReleaseAll:
		/// 	Releases all currently held InputBinds and invokes a release.
		///----------------------------------------------------------------------------------------------------
		void ReleaseAll();
	};
}
