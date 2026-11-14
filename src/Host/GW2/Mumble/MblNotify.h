///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblNotify.h
/// Description  :  Mumble identity notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

#include "thirdparty/mumble/Mumble.h"

#include "Engine/Notifier/Notifier.h"

constexpr int32_t MUMBLEIDENTITY_PRIO_SCALING = 0;
constexpr int32_t MUMBLEIDENTITY_PRIO_UI      = 100;

///----------------------------------------------------------------------------------------------------
/// IMumbleIdentityNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IMumbleIdentityNotifyHandler : public INotifyHandler<IMumbleIdentityNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnMumbleIdentityNotify:
	/// 	Called on the Mumble reader thread after the identity changed.
	///----------------------------------------------------------------------------------------------------
	virtual void OnMumbleIdentityNotify(const Mumble::Identity& aIdentity) = 0;
};

///----------------------------------------------------------------------------------------------------
/// IMumbleIdentityNotifier Interface
///----------------------------------------------------------------------------------------------------
class IMumbleIdentityNotifier : public INotifier<IMumbleIdentityNotifyHandler>
{
	protected:
	void NotifyMumbleIdentity(const Mumble::Identity& aIdentity)
	{
		for (const auto& entry : this->Subscribers())
		{
			entry.Subscriber->OnMumbleIdentityNotify(aIdentity);
		}
	}
};
