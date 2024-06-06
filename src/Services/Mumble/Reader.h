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
	~CMumbleReader() = default;
private:
	std::string			Name;
	std::thread			Thread;

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
