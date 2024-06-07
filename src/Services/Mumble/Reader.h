///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Reader.h
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MUMBLE_READER_H
#define MUMBLE_READER_H

#include <thread>
#include <string>

#include "Services/Mumble/Definitions/Mumble.h"

/* Log Channel*/
constexpr const char* CH_MUMBLE_READER	= "MumbleReader";

/* DataLink */
constexpr const char* DL_MUMBLE_LINK	= "DL_MUMBLE_LINK";
constexpr const char* DL_NEXUS_LINK		= "DL_NEXUS_LINK";

/* UI Scale */
constexpr const float SC_SMALL			= 0.90f;
constexpr const float SC_NORMAL			= 1.00f;
constexpr const float SC_LARGE			= 1.11f;
constexpr const float SC_LARGER			= 1.22f;

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
	bool			IsMumbleDisabled			= false;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CMumbleReader(std::string aMumbleName = "MumbleLink");

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CMumbleReader();
private:
	std::string			Name;
	std::thread			Thread;
	bool				IsRunning				= false;

	unsigned			PreviousTick			= 0;
	Vector3				PreviousAvatarPosition	= {};
	Vector3				PreviousCameraFront		= {};
	Mumble::Identity	PreviousIdentity		= {};
	long long			PreviousFrameCounter	= 0;

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Reader loop function.
	///----------------------------------------------------------------------------------------------------
	void Advance();
};

#endif
