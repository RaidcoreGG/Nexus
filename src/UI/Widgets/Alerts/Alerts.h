///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Alerts.h
/// Description  :  Contains the logic for the Alerts HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ALERTS_H
#define ALERTS_H

#include <mutex>
#include <string>
#include <vector>

#include "Engine/Loader/NexusLinkData.h"
#include "Engine/DataLink/DlApi.h"
#include "UI/Controls/CtlWindow.h"
#include "AlMessage.h"
#include "AlEnum.h"

///----------------------------------------------------------------------------------------------------
/// CAlerts Class
///----------------------------------------------------------------------------------------------------
class CAlerts : public virtual IWindow
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CAlerts(CDataLinkApi* aDataLink);

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

	NexusLinkData_t*            NexusLink;

	std::mutex                  Mutex;
	std::vector<AlertMessage_t> Queue;

	float                       Opacity = 1.0f;
};

#endif
