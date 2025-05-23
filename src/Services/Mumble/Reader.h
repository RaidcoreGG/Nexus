///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Reader.h
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MUMBLE_READER_H
#define MUMBLE_READER_H

#include <string>
#include <thread>

#include "Events/EvtApi.h"
#include "Loader/NexusLinkData.h"
#include "Services/DataLink/DlApi.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Mumble/Definitions/Mumble.h"

constexpr const char* CH_MUMBLE_READER = "MumbleReader";
constexpr const char* DL_MUMBLE_LINK = "DL_MUMBLE_LINK";
constexpr const char* DL_MUMBLE_LINK_IDENTITY = "DL_MUMBLE_LINK_IDENTITY";
constexpr const char* EV_MUMBLE_IDENTITY_UPDATED = "EV_MUMBLE_IDENTITY_UPDATED";

/* UI Scale */
constexpr const float SC_SMALL = 0.90f;
constexpr const float SC_NORMAL = 1.00f;
constexpr const float SC_LARGE = 1.11f;
constexpr const float SC_LARGER = 1.22f;

///----------------------------------------------------------------------------------------------------
/// Mumble Namespace
///----------------------------------------------------------------------------------------------------
namespace Mumble
{
	///----------------------------------------------------------------------------------------------------
	/// GetScalingFactor:
	/// 	Returns the scaling factor for the given the UISize enum.
	///----------------------------------------------------------------------------------------------------
	float GetScalingFactor(EUIScale aSize);
};

///----------------------------------------------------------------------------------------------------
/// CMumbleReader Class
///----------------------------------------------------------------------------------------------------
class CMumbleReader
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CMumbleReader(CDataLinkApi* aDataLink, CEventApi* aEventApi, CLogHandler* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CMumbleReader();

	///----------------------------------------------------------------------------------------------------
	/// GetName:
	/// 	Returns the name of the MumbleLink file.
	///----------------------------------------------------------------------------------------------------
	std::string GetName();

	///----------------------------------------------------------------------------------------------------
	/// IsDisabled:
	/// 	Returns whether the MumbleLink API is explicitly disabled.
	///----------------------------------------------------------------------------------------------------
	bool IsDisabled();

	private:
	CDataLinkApi*     DataLinkApi            = nullptr;
	CEventApi*        EventApi               = nullptr;
	CLogHandler*      Logger                 = nullptr;

	std::thread       Thread;
	bool              IsRunning              = false;

	std::string       Name;
	Mumble::Data*     MumbleLink             = nullptr;
	Mumble::Identity* MumbleIdentity         = nullptr;
	NexusLinkData*    NexusLink              = nullptr;

	bool              Flip                   = false;
	unsigned          PreviousTick           = 0;
	Vector3           PreviousAvatarPosition = {};
	Vector3           PreviousCameraFront    = {};
	Mumble::Identity  PreviousIdentity       = Mumble::Identity{};
	long long         PreviousFrameCounter   = 0;

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Reader loop function.
	///----------------------------------------------------------------------------------------------------
	void Advance();
};

#endif
