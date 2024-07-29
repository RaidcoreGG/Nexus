///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GameBindsHandler.h
/// Description  :  Provides functions for game InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef GAMEBINDSHANDLER_H
#define GAMEBINDSHANDLER_H

#include <filesystem>
#include <map>
#include <mutex>
#include <string>

#include "EGameBinds.h"

#include "Events/EventHandler.h"
#include "Inputs/InputBinds/InputBind.h"
#include "Inputs/RawInput/RawInputApi.h"
#include "Services/Logging/LogHandler.h"

constexpr const char* CH_GAMEBINDS = "GameBinds";
constexpr const char* EV_UE_KB_CH = "EV_UNOFFICIAL_EXTRAS_KEYBIND_CHANGED";

///----------------------------------------------------------------------------------------------------
/// GameBinds Namespace
///----------------------------------------------------------------------------------------------------
namespace GameBinds
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_PressAsync:
	/// 	Presses the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_PressAsync(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_ReleaseAsync:
	/// 	Releases the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_ReleaseAsync(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_InvokeAsync:
	/// 	Presses and releases the keys of a game bind.
	/// 	aDuration is the wait time in milliseconds between press and release.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_InvokeAsync(EGameBinds aGameBind, int aDuration);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Press:
	/// 	Presses the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_Press(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Release:
	/// 	Releases the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_Release(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_IsBound:
	/// 	Returns whether a game bind has a InputBind set or not.
	///----------------------------------------------------------------------------------------------------
	bool ADDONAPI_IsBound(EGameBinds aGameBind);
}

class CGameBindsApi
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ToString:
	/// 	Returns the unlocalized identifier of a game bind.
	///----------------------------------------------------------------------------------------------------
	static std::string ToString(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// GetCategory:
	/// 	Returns the unlocalized category of a game bind.
	///----------------------------------------------------------------------------------------------------
	static std::string GetCategory(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// GameBindCodeToScanCode:
	/// 	Converts a scan code from the game to a regular scan code.
	///----------------------------------------------------------------------------------------------------
	static unsigned short GameBindCodeToScanCode(unsigned short aGameScanCode);

	///----------------------------------------------------------------------------------------------------
	/// EnableUEInputBindUpdates:
	/// 	Enables Unofficial Extras InputBind updates for the passed API.
	///----------------------------------------------------------------------------------------------------
	static void EnableUEInputBindUpdates(CGameBindsApi* aGameBindsApi);

	///----------------------------------------------------------------------------------------------------
	/// DisableUEInputBindUpdates:
	/// 	Disables Unofficial Extras InputBind updates.
	///----------------------------------------------------------------------------------------------------
	static void DisableUEInputBindUpdates();

	///----------------------------------------------------------------------------------------------------
	/// OnUEInputBindChanged:
	/// 	Receives runtime InputBind updates from Unofficial Extras.
	///----------------------------------------------------------------------------------------------------
	static void OnUEInputBindChanged(void* aData);

	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CGameBindsApi(CRawInputApi* aRawInputApi, CLogHandler* aLogger, CEventApi* aEventApi);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CGameBindsApi();

	///----------------------------------------------------------------------------------------------------
	/// PressAsync:
	/// 	Presses the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void PressAsync(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// ReleaseAsync:
	/// 	Releases the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void ReleaseAsync(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// InvokeAsync:
	/// 	Presses and releases the keys of a game bind.
	/// 	aDuration is the wait time in milliseconds between press and release.
	///----------------------------------------------------------------------------------------------------
	void InvokeAsync(EGameBinds aGameBind, int aDuration);

	///----------------------------------------------------------------------------------------------------
	/// Press:
	/// 	Presses the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void Press(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// Release:
	/// 	Releases the keys of a game bind.
	///----------------------------------------------------------------------------------------------------
	void Release(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// IsBound:
	/// 	Returns whether a game bind has a InputBind set or not.
	///----------------------------------------------------------------------------------------------------
	bool IsBound(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// Import:
	/// 	Imports the game binds from an Inputs.xml.
	///----------------------------------------------------------------------------------------------------
	void Import(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Gets a game bind.
	///----------------------------------------------------------------------------------------------------
	InputBind Get(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Sets a game bind.
	///----------------------------------------------------------------------------------------------------
	void Set(EGameBinds aGameBind, InputBind aInputBind, bool aIsRuntimeBind = false);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::map<EGameBinds, InputBind> GetRegistry() const;

	private:
	CRawInputApi*					RawInputApi;
	CLogHandler*					Logger;
	CEventApi*						EventApi;

	mutable std::mutex				Mutex;
	std::map<EGameBinds, InputBind>	Registry;

	bool							IsReceivingRuntimeBinds;

	///----------------------------------------------------------------------------------------------------
	/// AddDefaultBinds:
	/// 	Adds the default binds, if they don't already exist.
	///----------------------------------------------------------------------------------------------------
	void AddDefaultBinds();

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the saved game binds from disk.
	///----------------------------------------------------------------------------------------------------
	void Load();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the game binds to disk.
	///----------------------------------------------------------------------------------------------------
	void Save();
};

#endif
