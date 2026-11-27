///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclNotify.h
/// Description  :  Game language notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Engine/Notifier/Notifier.h"

namespace Raidcore::UI
{
	///----------------------------------------------------------------------------------------------------
	/// IGameLanguageNotifyHandler Interface
	///----------------------------------------------------------------------------------------------------
	class IGameLanguageNotifyHandler : public Engine::INotifyHandler<IGameLanguageNotifyHandler>
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// OnGameLanguageNotify:
		/// 	An external source reports the game language, as a locale identifier (e.g. "en").
		///----------------------------------------------------------------------------------------------------
		virtual void OnGameLanguageNotify(const char* aLanguage) = 0;
	};

	///----------------------------------------------------------------------------------------------------
	/// IGameLanguageNotifier Interface
	///----------------------------------------------------------------------------------------------------
	class IGameLanguageNotifier : public Engine::INotifier<IGameLanguageNotifyHandler>
	{
		protected:
		void NotifyGameLanguage(const char* aLanguage)
		{
			for (const auto& entry : this->Subscribers())
			{
				entry.Subscriber->OnGameLanguageNotify(aLanguage);
			}
		}
	};
}
