///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Reader.cpp
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Services/Mumble/Reader.h"

#include "Consts.h"
#include "Shared.h" /* FIXME: This is included at the moment because the LogHandler isn't dependency-injected. Fix before 2024/06/30. */
#include "Renderer.h"
#include "Hooks.h" /* FIXME: This is included at the moment because the NexusLink isn't dependency-injected. Fix before 2024/06/30. */

#include "Events/EventHandler.h"
#include "Services/DataLink/DataLink.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

using namespace Mumble;

namespace Mumble
{
	Identity* IdentityParsed = new Identity{};
	volatile bool SuppressExcessiveParseErrors = false;
	volatile unsigned int ParserErrorCount = 0;

	bool operator==(const Identity& lhs, const Identity& rhs)
	{
		if (strcmp(lhs.Name, rhs.Name) == 0 &&
			lhs.Profession == rhs.Profession &&
			lhs.Specialization == rhs.Specialization &&
			lhs.Race == rhs.Race &&
			lhs.MapID == rhs.MapID &&
			lhs.WorldID == rhs.WorldID &&
			lhs.TeamColorID == rhs.TeamColorID &&
			lhs.IsCommander == rhs.IsCommander &&
			lhs.FOV == rhs.FOV &&
			lhs.UISize == rhs.UISize)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Identity& lhs, const Identity& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const Vector2& lhs, const Vector2& rhs)
	{
		if (trunc(1000. * lhs.X) == trunc(1000. * rhs.X) &&
			trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y))
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Vector2& lhs, const Vector2& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const Vector3& lhs, const Vector3& rhs)
	{
		if (trunc(1000. * lhs.X) == trunc(1000. * rhs.X) &&
			trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y) &&
			trunc(1000. * lhs.Z) == trunc(1000. * rhs.Z))
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Vector3& lhs, const Vector3& rhs)
	{
		return !(lhs == rhs);
	}

	float GetScalingFactor(EUIScale aSize)
	{
		switch (aSize)
		{
		case EUIScale::Small:	return SC_SMALL;	// Small
		default:
		case EUIScale::Normal:	return SC_NORMAL;	// Normal
		case EUIScale::Large:	return SC_LARGE;	// Large
		case EUIScale::Larger:	return SC_LARGER;	// Larger
		}
	}
}

CMumbleReader::CMumbleReader(std::string aMumbleName)
{
	this->Name = aMumbleName;

	/* share the linked mem regardless whether it's disabled, for dependant addons */
	MumbleLink = (Mumble::Data*)DataLinkService->ShareResource(DL_MUMBLE_LINK, sizeof(Mumble::Data), aMumbleName.c_str(), true);
	Hooks::NexusLink = NexusLink = (NexusLinkData*)DataLinkService->ShareResource(DL_NEXUS_LINK, sizeof(NexusLinkData), true);

	if (aMumbleName == "0")
	{
		this->IsMumbleDisabled = true;
	}
	else
	{
		this->IsRunning = true;
		this->Thread = std::thread(&CMumbleReader::Advance, this);
	}
}

CMumbleReader::~CMumbleReader()
{
	this->IsRunning = false;

	if (this->Thread.joinable())
	{
		this->Thread.join();
	}
}

void CMumbleReader::Advance()
{
	constexpr unsigned int MAX_ERRORS = 5;

	while (this->IsRunning)
	{
		this->Flip = !this->Flip; // every other tick so it gets refreshed every 100ms
		if (this->Flip)
		{
			this->NexusLink->IsGameplay = this->PreviousTick != this->MumbleLink->UITick ||
				(this->PreviousFrameCounter == Renderer::FrameCounter && this->NexusLink->IsGameplay);
			this->NexusLink->IsMoving = this->PreviousAvatarPosition != this->MumbleLink->AvatarPosition;
			this->NexusLink->IsCameraMoving = this->PreviousCameraFront != this->MumbleLink->CameraFront;

			this->PreviousFrameCounter = Renderer::FrameCounter;
			this->PreviousTick = this->MumbleLink->UITick;
			this->PreviousAvatarPosition = this->MumbleLink->AvatarPosition;
			this->PreviousCameraFront = this->MumbleLink->CameraFront;
		}

		if (this->MumbleLink->Identity[0])
		{
			/* cache identity */
			this->PreviousIdentity = *IdentityParsed;

			try
			{
				/* parse and assign current identity */
				json j = json::parse(MumbleLink->Identity);
				strcpy(IdentityParsed->Name, j["name"].get<std::string>().c_str());
				j["profession"].get_to(IdentityParsed->Profession);
				j["spec"].get_to(IdentityParsed->Specialization);
				j["race"].get_to(IdentityParsed->Race);
				j["map_id"].get_to(IdentityParsed->MapID);
				j["world_id"].get_to(IdentityParsed->WorldID);
				j["team_color_id"].get_to(IdentityParsed->TeamColorID);
				j["commander"].get_to(IdentityParsed->IsCommander);
				j["fov"].get_to(IdentityParsed->FOV);
				j["uisz"].get_to(IdentityParsed->UISize);
			}
			catch (json::parse_error& ex)
			{
				if(!SuppressExcessiveParseErrors || ParserErrorCount < MAX_ERRORS)
					Logger->Trace(CH_MUMBLE_READER, "MumbleLink could not be parsed. Parse Error: %s", ex.what());
				else if(ParserErrorCount == MAX_ERRORS)
					Logger->Trace(CH_MUMBLE_READER, "Suppressing further errors");
				ParserErrorCount++;
			}
			catch (json::type_error& ex)
			{
				if(!SuppressExcessiveParseErrors || ParserErrorCount < MAX_ERRORS)
					Logger->Trace(CH_MUMBLE_READER, "MumbleLink could not be parsed. Type Error: %s", ex.what());
				else if(ParserErrorCount == MAX_ERRORS)
					Logger->Trace(CH_MUMBLE_READER, "Suppressing further errors");
				ParserErrorCount++;
			}
			catch (...)
			{
				if(!SuppressExcessiveParseErrors || ParserErrorCount < MAX_ERRORS)
					Logger->Trace(CH_MUMBLE_READER, "MumbleLink could not be parsed. Unknown Error.");
				else if(ParserErrorCount == MAX_ERRORS)
					Logger->Trace(CH_MUMBLE_READER, "Suppressing further errors");
				ParserErrorCount++;
			}

			/* notify (also notifies the GUI to update its scaling factor) */
			if (*IdentityParsed != this->PreviousIdentity)
			{
				EventApi->Raise("EV_MUMBLE_IDENTITY_UPDATED", IdentityParsed);
			}
		}
		for (size_t i = 0; i < 50; i++)
		{
			if (!this->IsRunning) { return; } // abort if shutdown during sleep

			Sleep(1);
		}
	}
}
