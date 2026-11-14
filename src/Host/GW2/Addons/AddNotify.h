///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddNotify.h
/// Description  :  Addon state notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

#include "Engine/Notifier/Notifier.h"

constexpr int32_t ADDONSTATE_PRIO_MAINWINDOW   = 0;
constexpr int32_t ADDONSTATE_PRIO_ADDONSWINDOW = 100;
constexpr int32_t ADDONSTATE_PRIO_QUICKACCESS  = 200;

///----------------------------------------------------------------------------------------------------
/// EAddonStateNotify Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonStateNotify
{
	Created,
	Destroyed,
	Loaded,
	Unloaded,
	VolatileDisabled /* The load was canceled, the game was updated since the addon last loaded. */
};

///----------------------------------------------------------------------------------------------------
/// IAddonStateNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IAddonStateNotifyHandler : public INotifyHandler<IAddonStateNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnAddonStateNotify:
	/// 	Called on the addon's thread. aSignature is 0 if the addon has none yet.
	///----------------------------------------------------------------------------------------------------
	virtual void OnAddonStateNotify(EAddonStateNotify aState, uint32_t aSignature) = 0;
};

///----------------------------------------------------------------------------------------------------
/// IAddonStateNotifier Interface
///----------------------------------------------------------------------------------------------------
class IAddonStateNotifier : public INotifier<IAddonStateNotifyHandler>
{
	protected:
	void NotifyAddonState(EAddonStateNotify aState, uint32_t aSignature)
	{
		for (const auto& entry : this->Subscribers())
		{
			entry.Subscriber->OnAddonStateNotify(aState, aSignature);
		}
	}
};
