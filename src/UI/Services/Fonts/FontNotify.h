///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontNotify.h
/// Description  :  Font update notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "imgui/imgui.h"

#include "Engine/Notifier/Notifier.h"

namespace Raidcore::UI
{
	///----------------------------------------------------------------------------------------------------
	/// IFontUpdateNotifyHandler Interface
	///----------------------------------------------------------------------------------------------------
	class IFontUpdateNotifyHandler : public Engine::INotifyHandler<IFontUpdateNotifyHandler>
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// OnFontUpdateNotify:
		/// 	Called on the render thread for every font whose pointer changed, for any font, including
		/// 	addon fonts. First with nullptr before the previous atlases are freed, then with the new font.
		///----------------------------------------------------------------------------------------------------
		virtual void OnFontUpdateNotify(const char* aIdentifier, ImFont* aFont) = 0;
	};

	///----------------------------------------------------------------------------------------------------
	/// IFontUpdateNotifier Interface
	///----------------------------------------------------------------------------------------------------
	class IFontUpdateNotifier : public Engine::INotifier<IFontUpdateNotifyHandler>
	{
		protected:
		void NotifyFontUpdate(const char* aIdentifier, ImFont* aFont)
		{
			for (const auto& entry : this->Subscribers())
			{
				entry.Subscriber->OnFontUpdateNotify(aIdentifier, aFont);
			}
		}
	};
}
