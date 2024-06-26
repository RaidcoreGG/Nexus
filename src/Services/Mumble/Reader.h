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
#include "Loader/NexusLinkData.h"

/* Log Channel*/
constexpr const char* CH_MUMBLE_READER				= "MumbleReader";

/* UI Scale */
constexpr const float SC_SMALL						= 0.90f;
constexpr const float SC_NORMAL						= 1.00f;
constexpr const float SC_LARGE						= 1.11f;
constexpr const float SC_LARGER						= 1.22f;

///----------------------------------------------------------------------------------------------------
/// Mumble Namespace
///----------------------------------------------------------------------------------------------------
namespace Mumble
{
	extern Identity* IdentityParsed;
	extern volatile bool SuppressExcessiveParseErrors;
	extern volatile unsigned int ParserErrorCount;

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
	std::thread			Thread;
	bool				IsRunning				= false;

	std::string			Name;
	Mumble::Data*		MumbleLink				= nullptr;
	NexusLinkData*		NexusLink				= nullptr;

	bool				Flip					= false;
	unsigned			PreviousTick			= 0;
	Vector3				PreviousAvatarPosition	= {};
	Vector3				PreviousCameraFront		= {};
	Mumble::Identity	PreviousIdentity		= Mumble::Identity{};
	long long			PreviousFrameCounter	= 0;

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Reader loop function.
	///----------------------------------------------------------------------------------------------------
	void Advance();
};

#endif
