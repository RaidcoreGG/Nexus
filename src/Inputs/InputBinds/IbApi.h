///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbApi.h
/// Description  :  Provides functions for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBAPI_H
#define IBAPI_H

#include <map>
#include <mutex>
#include <string>
#include <windows.h>

#include "Events/EvtApi.h"
#include "IbFuncDefs.h"
#include "IbBindV2.h"
#include "IbMapping.h"
#include "IbCapture.h"
#include "Services/Logging/LogApi.h"

constexpr const char* CH_INPUTBINDS = "InputBinds";

///----------------------------------------------------------------------------------------------------
/// CInputBindApi Class
///----------------------------------------------------------------------------------------------------
class CInputBindApi : public CInputBindCapture
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CInputBindApi(CEventApi* aEventApi, CLogApi* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if a InputBind was invoked.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Generates and registers a InputBind from the given string with the given identifier and handler,
	/// 	if no bind was previously stored the given one will be used.
	///----------------------------------------------------------------------------------------------------
	void Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, const char* aInputBind);

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers a InputBind from the given struct with the given identifier and handler,
	/// 	if no bind was previously stored the given one will be used.
	///----------------------------------------------------------------------------------------------------
	void Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, InputBind aInputBind);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters a InputBindHandler from an identifier.
	///----------------------------------------------------------------------------------------------------
	void Deregister(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// IsInUse:
	/// 	Returns an empty string if InputBind is unused or the identifier that uses this InputBind.
	///----------------------------------------------------------------------------------------------------
	std::string IsInUse(InputBind aInputBind);
	
	///----------------------------------------------------------------------------------------------------
	/// HasHandler:
	/// 	Returns true if the InputBind with the passed identifier has a handler registered.
	///----------------------------------------------------------------------------------------------------
	bool HasHandler(const std::string& aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Gets a InputBind.
	///----------------------------------------------------------------------------------------------------
	const InputBind& Get(const std::string& aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Sets a InputBind.
	///----------------------------------------------------------------------------------------------------
	void Set(std::string aIdentifier, InputBind aInputBind);

	///----------------------------------------------------------------------------------------------------
	/// SetPassthrough:
	/// 	Sets the passthrough state of an InputBind.
	///----------------------------------------------------------------------------------------------------
	void SetPassthrough(std::string aIdentifier, bool aPassthrough);

	///----------------------------------------------------------------------------------------------------
	/// Invoke:
	/// 	Invokes the action on the corresponding InputBind handler.
	/// 	Returns true if the InputBind was dispatched and processing should not continue.
	///----------------------------------------------------------------------------------------------------
	bool Invoke(std::string aIdentifier, bool aIsRelease = false);

	///----------------------------------------------------------------------------------------------------
	/// Deletes:
	/// 	Deletes a InputBind entirely.
	///----------------------------------------------------------------------------------------------------
	void Delete(std::string aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all InputBindHandlers that are within the provided address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, IbMapping> GetRegistry() const;

	private:
	CEventApi*                       EventApi = nullptr;
	CLogApi*                         Logger   = nullptr;

	mutable std::mutex               Mutex;
	std::map<std::string, IbMapping> Registry;
	std::map<std::string, IbMapping> HeldInputBinds;

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
	/// 	Invokes an InputBind that matches the pressed inputs.
	/// 	Returns true if a bind was found and invoked.
	///----------------------------------------------------------------------------------------------------
	bool Press(const InputBind& aInputBind);

	///----------------------------------------------------------------------------------------------------
	/// Release:
	/// 	Releases all InputBinds matching the criteria and invokes a release.
	///----------------------------------------------------------------------------------------------------
	void Release(unsigned int aModifierVK);

	///----------------------------------------------------------------------------------------------------
	/// Release:
	/// 	Releases all InputBinds matching the criteria and invokes a release.
	///----------------------------------------------------------------------------------------------------
	void Release(EInputDevice aDevice, unsigned short aCode);

	///----------------------------------------------------------------------------------------------------
	/// ReleaseAll:
	/// 	Releases all currently held InputBinds and invokes a release.
	///----------------------------------------------------------------------------------------------------
	void ReleaseAll();
};

#endif
