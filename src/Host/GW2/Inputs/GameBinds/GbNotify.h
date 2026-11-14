///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbNotify.h
/// Description  :  Game bind changed notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Engine/Inputs/InputBinds/IbBindV2.h"
#include "Engine/Notifier/Notifier.h"
#include "GbEnum.h"

///----------------------------------------------------------------------------------------------------
/// IGameBindChangedNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IGameBindChangedNotifyHandler : public INotifyHandler<IGameBindChangedNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnGameBindChangedNotify:
	/// 	An external source reports that a game bind was changed in the game.
	///----------------------------------------------------------------------------------------------------
	virtual void OnGameBindChangedNotify(EGameBinds aGameBind, const InputBind_t& aInputBind, bool aIsPrimary) = 0;
};

///----------------------------------------------------------------------------------------------------
/// IGameBindChangedNotifier Interface
///----------------------------------------------------------------------------------------------------
class IGameBindChangedNotifier : public INotifier<IGameBindChangedNotifyHandler>
{
	protected:
	void NotifyGameBindChanged(EGameBinds aGameBind, const InputBind_t& aInputBind, bool aIsPrimary)
	{
		for (const auto& entry : this->Subscribers())
		{
			entry.Subscriber->OnGameBindChangedNotify(aGameBind, aInputBind, aIsPrimary);
		}
	}
};
