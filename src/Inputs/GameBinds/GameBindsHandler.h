///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GameBindsHandler.h
/// Description  :  Provides functions for game keybinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef GAMEBINDSHANDLER_H
#define GAMEBINDSHANDLER_H

#include <filesystem>
#include <map>
#include <mutex>

#include "EGameBinds.h"

#include "Inputs/RawInput/RawInputApi.h"
#include "Inputs/Keybinds/Keybind.h"

#include "Events/EventHandler.h"

#include "Services/Logging/LogHandler.h"

constexpr const char* CH_GAMEBINDS = "GameBinds";
constexpr const char* EV_UE_KB_CH = "EV_UNOFFICIAL_EXTRAS_KEYBIND_CHANGED";

class CGameBindsApi
{
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
	/// 	Returns whether a game bind has a keybind set or not.
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
	Keybind Get(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Sets a game bind.
	///----------------------------------------------------------------------------------------------------
	void Set(EGameBinds aGameBind, Keybind aKeybind, bool aIsRuntimeBind = false);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::map<EGameBinds, Keybind> GetRegistry() const;

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
	/// EnableUEKeybindUpdates:
	/// 	Enables Unofficial Extras keybind updates for the passed API.
	///----------------------------------------------------------------------------------------------------
	static void EnableUEKeybindUpdates(CGameBindsApi* aGameBindsApi);

	///----------------------------------------------------------------------------------------------------
	/// DisableUEKeybindUpdates:
	/// 	Disables Unofficial Extras keybind updates.
	///----------------------------------------------------------------------------------------------------
	static void DisableUEKeybindUpdates();

	///----------------------------------------------------------------------------------------------------
	/// OnUEKeybindChanged:
	/// 	Receives runtime keybind updates from Unofficial Extras.
	///----------------------------------------------------------------------------------------------------
	static void OnUEKeybindChanged(void* aData);

private:
	CRawInputApi*					RawInputApi;
	CLogHandler*					Logger;
	CEventApi*						EventApi;

	mutable std::mutex				Mutex;
	std::map<EGameBinds, Keybind>	Registry;

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
