///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbApi.h
/// Description  :  Provides functions for game InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <unordered_map>
#include <mutex>

#include "GbEnum.h"

#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/InputBinds/IbBindV2.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Logging/LogApi.h"

constexpr const char* CH_GAMEBINDS = "GameBinds";
constexpr const char* EV_UE_KB_CH = "EV_UNOFFICIAL_EXTRAS_KEYBIND_CHANGED";

#define WM_PASSTHROUGH_FIRST WM_USER + 7997
#define WM_PASSTHROUGH_LAST  WM_USER + 7997 + WM_USER - 1

///----------------------------------------------------------------------------------------------------
/// MultiInputBind_t Struct
///----------------------------------------------------------------------------------------------------
struct MultiInputBind_t
{
	InputBind_t Primary;
	InputBind_t Secondary;
};

///----------------------------------------------------------------------------------------------------
/// CGameBindsApi Class
///----------------------------------------------------------------------------------------------------
class CGameBindsApi
{
	public:
	// FIXME: this needs to be moved to an evtconn
	///----------------------------------------------------------------------------------------------------
	/// OnUEInputBindChanged:
	/// 	Receives runtime InputBind_t updates from Unofficial Extras.
	///----------------------------------------------------------------------------------------------------
	static void OnUEInputBindChanged(void* aData);

	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CGameBindsApi(
		CRawInputApi*         aRawInputApi,
		CLogApi*              aLogger,
		CEventApi*            aEventApi,
		RenderContext_t*      aRenderContext,
		std::filesystem::path aConfigPath
	);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CGameBindsApi();

	///----------------------------------------------------------------------------------------------------
	/// RedirectGameOnly:
	/// 	Returns the uMsg shifted back to the normal range.
	/// 	This should be called after all other window procedures.
	///----------------------------------------------------------------------------------------------------
	UINT RedirectGameOnly(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam);

	///----------------------------------------------------------------------------------------------------
	/// SendWndProcToGame:
	/// 	Skips all WndProc callbacks and sends it directly to the original.
	///----------------------------------------------------------------------------------------------------
	LRESULT SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
	/// RestoreModifiers:
	/// 	Restores the modifiers as polled from the system.
	///----------------------------------------------------------------------------------------------------
	void RestoreModifiers();

	///----------------------------------------------------------------------------------------------------
	/// IsBound:
	/// 	Returns whether a game bind has a InputBind_t set or not.
	///----------------------------------------------------------------------------------------------------
	bool IsBound(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Gets a game bind.
	/// 	Returns nullptr, if it doesn't exist.
	///----------------------------------------------------------------------------------------------------
	const MultiInputBind_t* Get(EGameBinds aGameBind);

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Sets a game bind.
	///----------------------------------------------------------------------------------------------------
	void Set(EGameBinds aGameBind, InputBind_t aInputBind, bool aIsPrimary, bool aIsRuntimeBind);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::unordered_map<EGameBinds, MultiInputBind_t> GetRegistry() const;

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the saved game binds from disk.
	///----------------------------------------------------------------------------------------------------
	void Load(std::filesystem::path aPath);

	private:
	CRawInputApi*                                    RawInputApi;
	CLogApi*                                         Logger;
	CEventApi*                                       EventApi;
	RenderContext_t*                                 RenderContext;

	std::filesystem::path                            ConfigPath;

	mutable std::mutex                               Mutex;
	std::unordered_map<EGameBinds, MultiInputBind_t> Registry;

	///----------------------------------------------------------------------------------------------------
	/// AddDefaultBinds:
	/// 	Adds the default binds, if they don't already exist.
	///----------------------------------------------------------------------------------------------------
	void AddDefaultBinds();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the game binds to disk.
	///----------------------------------------------------------------------------------------------------
	void Save();
};
