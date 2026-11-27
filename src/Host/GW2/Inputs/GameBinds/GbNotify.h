///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbNotify.h
/// Description  :  Game bind changed notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Engine/Inputs/InputBinds/IbBind.h"
#include "Engine/Notifier/Notifier.h"
#include "GbEnum.h"

namespace Raidcore::GW2
{
	///----------------------------------------------------------------------------------------------------
	/// IGameBindChangedNotifyHandler Interface
	///----------------------------------------------------------------------------------------------------
	class IGameBindChangedNotifyHandler : public Engine::INotifyHandler<IGameBindChangedNotifyHandler>
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// OnGameBindChangedNotify:
		/// 	An external source reports that a game bind was changed in the game.
		///----------------------------------------------------------------------------------------------------
		virtual void OnGameBindChangedNotify(EGameBinds aGameBind, const Engine::Input::Bind_t& aInputBind, bool aIsPrimary) = 0;
	};

	///----------------------------------------------------------------------------------------------------
	/// IGameBindChangedNotifier Interface
	///----------------------------------------------------------------------------------------------------
	class IGameBindChangedNotifier : public Engine::INotifier<IGameBindChangedNotifyHandler>
	{
		protected:
		void NotifyGameBindChanged(EGameBinds aGameBind, const Engine::Input::Bind_t& aInputBind, bool aIsPrimary)
		{
			for (const auto& entry : this->Subscribers())
			{
				entry.Subscriber->OnGameBindChangedNotify(aGameBind, aInputBind, aIsPrimary);
			}
		}
	};
}
