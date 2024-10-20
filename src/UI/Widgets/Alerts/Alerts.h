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

#include "Loader/NexusLinkData.h"
#include "Services/DataLink/DataLink.h"

///----------------------------------------------------------------------------------------------------
/// CAlerts Class
///----------------------------------------------------------------------------------------------------
class CAlerts
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CAlerts(CDataLink* aDataLink);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CAlerts();

	///----------------------------------------------------------------------------------------------------
	/// Render:
	/// 	Renders the Alerts.
	///----------------------------------------------------------------------------------------------------
	void Render();

	///----------------------------------------------------------------------------------------------------
	/// Notify:
	/// 	Queues a new alert.
	///----------------------------------------------------------------------------------------------------
	void Notify(const char* aMessage);

	private:
	struct Alert
	{
		std::string    Message;
		double         StartTime  = 0;
	};

	NexusLinkData*     NexusLink;

	std::mutex         Mutex;
	std::vector<Alert> Queue;

	float              Opacity    = 1.0f;
};

#endif
