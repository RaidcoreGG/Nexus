///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Alerts.h
/// Description  :  Contains the logic for the Alerts HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "AlEnum.h"
#include "AlMessage.h"
#include "Host/GW2/Addons/API/NexusLink.h"
#include "Engine/DataLink/DlApi.h"
#include "UI/Controls/CtlWindow.h"

namespace Raidcore::UI
{
	///----------------------------------------------------------------------------------------------------
	/// CAlerts Class
	///----------------------------------------------------------------------------------------------------
	class CAlerts : public virtual UI::IWindow
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		CAlerts(Engine::CDataLinkApi* aDataLink);

		///----------------------------------------------------------------------------------------------------
		/// Render:
		/// 	Renders the Alerts.
		///----------------------------------------------------------------------------------------------------
		void Render() override;

		///----------------------------------------------------------------------------------------------------
		/// Notify:
		/// 	Queues a new alert.
		///----------------------------------------------------------------------------------------------------
		void Notify(EAlertType aType, const char* aMessage);

		private:

		GW2::NexusLinkData_t*            NexusLink;

		std::mutex                  Mutex;
		std::vector<AlertMessage_t> Queue;

		float                       Opacity = 1.0f;
	};
}
