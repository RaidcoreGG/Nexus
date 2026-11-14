///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbNotify.h
/// Description  :  Input bind update notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

#include "Engine/Notifier/Notifier.h"

constexpr int32_t INPUTBINDUPDATE_PRIO_UI = 0;

///----------------------------------------------------------------------------------------------------
/// IInputBindUpdateNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IInputBindUpdateNotifyHandler : public INotifyHandler<IInputBindUpdateNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnInputBindUpdateNotify:
	/// 	Called after an input bind or game bind was set, registered or deleted.
	///----------------------------------------------------------------------------------------------------
	virtual void OnInputBindUpdateNotify() = 0;
};

///----------------------------------------------------------------------------------------------------
/// IInputBindUpdateNotifier Interface
///----------------------------------------------------------------------------------------------------
class IInputBindUpdateNotifier : public INotifier<IInputBindUpdateNotifyHandler>
{
	protected:
	void NotifyInputBindUpdate()
	{
		for (const auto& entry : this->Subscribers())
		{
			entry.Subscriber->OnInputBindUpdateNotify();
		}
	}
};
