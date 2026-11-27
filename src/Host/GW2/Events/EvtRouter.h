///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtRouter.h
/// Description  :  Routes events raised by other addons to engine notifications.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Core/Logging/LogChannel.h"
#include "Engine/Events/EvtApi.h"
#include "Host/GW2/Inputs/GameBinds/GbNotify.h"
#include "UI/Services/Localization/LoclNotify.h"

namespace Input = Engine::Input;

constexpr const char* EV_UE_KB_CH       = "EV_UNOFFICIAL_EXTRAS_KEYBIND_CHANGED";
constexpr const char* EV_UE_LANGUAGE_CH = "EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED";

///----------------------------------------------------------------------------------------------------
/// CEventRouter Class
/// 	Owns every subscription to events from other addons. Validates their payloads and forwards them
/// 	as notifications, so components never subscribe to external events themselves.
///----------------------------------------------------------------------------------------------------
class CEventRouter
	: public ILogChannel
	, public IGameBindChangedNotifier
	, public IGameLanguageNotifier
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CEventRouter(CEventApi* aEventApi);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CEventRouter();

	private:
	CEventApi* EventApi = nullptr;

	///----------------------------------------------------------------------------------------------------
	/// OnUEKeybindChanged:
	/// 	Unofficial Extras: a game bind was changed in the game.
	///----------------------------------------------------------------------------------------------------
	static void OnUEKeybindChanged(void* aEventData);

	///----------------------------------------------------------------------------------------------------
	/// OnUELanguageChanged:
	/// 	Unofficial Extras: the game language changed.
	///----------------------------------------------------------------------------------------------------
	static void OnUELanguageChanged(void* aEventData);
};
